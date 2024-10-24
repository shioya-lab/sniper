#include "stream_prefetcher.h"
#include "simulator.h"
#include "config.hpp"

#include <cstdlib>

const IntPtr PAGE_SIZE = 4096;
const IntPtr PAGE_MASK = ~(PAGE_SIZE-1);

#define MYLOG(...) { if (m_enable_log && (m_pref_target_log == 0 || m_log_current_pc == m_pref_target_log)) { \
         fprintf(stderr, "%s:StreamPrefetcher ", configName.c_str()); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } }

StreamPrefetcher::StreamPrefetcher(String configName, core_id_t _core_id, UInt32 _shared_cores)
   : core_id(_core_id)
   , shared_cores(_shared_cores)
   , configName(configName)
   , m_cache_block_size      (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/cache_block_size", core_id))
   , m_monitor_window_size   (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/stream/monitor_window", core_id))
   , m_degree                (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/stream/degree", core_id))
   , m_stream_table_size     (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/stream/table_size", core_id))
   , m_training_window_size  (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/stream/training_window", core_id))
   , m_training_threshold    (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/stream/training_threshold", core_id))
   , m_enable_log            (Sim()->getCfg()->getBoolArray("log/enable_stream_prefetcher_log", core_id))
   , m_pref_target_log(strtol(Sim()->getCfg()->getStringArray("log/vec_pref_target_pc", core_id).c_str(), NULL, 16))
{
    // if(phase == INIT_PRE_CONNECTION){
    // Setup remaining members from loaded parameters.
    m_effectiveMonitorWindow  = m_cache_block_size * m_monitor_window_size;
    m_effectiveTrainingWindow = m_cache_block_size * m_training_window_size;
    LOG_ASSERT_ERROR (m_effectiveMonitorWindow != 0, "Invalid m_effectiveMonitorWindow");
    LOG_ASSERT_ERROR (m_effectiveTrainingWindow != 0, "Invalid m_effectiveTrainingWindow");

    m_hist_id = 0;
    // }
}


// Returns whether the 'addr' is in a window specified
// by the remaining arguments or not. The 'ascending' means
// a direction of a window.
bool StreamPrefetcher::is_in_window( IntPtr addr, IntPtr start, IntPtr windowSize, bool ascending)
{
    IntPtr line_addr  = MaskLineOffset(addr);
    IntPtr line_start = MaskLineOffset(start);

    // MYLOG ("is_in_window::Trying to check : line_addr=%08lx, line_start=%08lx, window=%08lx, ascending=%d",
    //        line_addr, line_start, windowSize, ascending);

    if (ascending) {
        return line_start <= line_addr && line_addr < line_start + windowSize;
    } else{
        return line_start - windowSize <= line_addr && line_addr < line_start;
    }
}



// Masks offset bits of the cache line in the 'addr'.
IntPtr StreamPrefetcher::MaskLineOffset( IntPtr addr)
{
   return addr & ~(m_cache_block_size - 1);
}

std::vector<IntPtr>
StreamPrefetcher::getNextAddress(IntPtr current_address, Core::mem_op_t mem_op_type, IntPtr pc, uint64_t uop_idx, core_id_t _core_id)
{
   if (pc == 0) {
      // プリフェッチャのリクエストそのものであれば、学習に使用しない
      return std::vector<IntPtr>();
   }

    m_log_current_pc = pc;

    MYLOG ("getNextAddress(pc=%08lx, adrdr=%08lx) start, size = %ld :", pc, current_address, m_stream_table.size());
    std::vector<IntPtr> addresses;

    auto monitor_ret = UpdateMonitorStream(current_address);
    bool streamTableHit = monitor_ret.first;
    if (streamTableHit) {
        size_t table_index = monitor_ret.second;
        auto stream = m_stream_table[table_index];

        LOG_ASSERT_ERROR (m_degree >= 3, "m_degree must larger than 2");

        IntPtr prefetch = stream->addr + (stream->ascending ? m_effectiveMonitorWindow : -m_effectiveMonitorWindow);
        for (int i = 0; i < m_degree; i++) {
           MYLOG("pushing address %08lx", MaskLineOffset(prefetch));
           addresses.push_back(MaskLineOffset(prefetch));
           prefetch = prefetch + (stream->ascending ? m_cache_block_size : -m_cache_block_size);
           stream->addr += m_cache_block_size;
        }
        MYLOG("Next stream start address = %08lx", MaskLineOffset(stream->addr));
    } else {
       if (UpdateTrainingStream(current_address)) {
          streamTableHit = true;
       }

       if (!streamTableHit && !isInEntryRegion (current_address)) {
          AllocateStream(current_address);
       }
    }

    return addresses;
}


// Check and update entries with MONITOR status in the stream table.
// Returns whether any entries are updated or not.
std::pair<bool, size_t> StreamPrefetcher::UpdateMonitorStream(IntPtr current_address)
{
    IntPtr miss_block_address = MaskLineOffset( current_address);

    MYLOG ("UpdateMonitorStream(%ld)::current_address = %08lx, miss_block_address = %08lx",
           m_stream_table.size(), miss_block_address, current_address);

    // Monitor
    for (size_t i = 0; i < m_stream_table.size(); i++){
        Stream* stream = m_stream_table[i];

        MYLOG ("UpdateMonitorStream: target_address = %08lx, stream_table[%ld] status=%d, orig=0x%08lx, start=0x%08lx, ascending=%d, count=%d hist_id=%ld window=0x%lx",
               miss_block_address, i, static_cast<int>(stream->status), stream->orig, stream->addr, stream->ascending, stream->count, stream->hist_id, m_effectiveMonitorWindow);

        if (stream->status != SS_MONITOR)
            continue;

        // Check a missed address is in a prefetch window.
        if (!is_in_window( miss_block_address, stream->addr, m_effectiveMonitorWindow, stream->ascending)) {
            continue;
        }

        MYLOG ("UpdateMonitorStream::current_address hit = %08lx, entry index = %ld, entry.addr=%08lx, entry.ascending=%d",
               current_address, i, stream->addr, stream->ascending);

        // Update the table
        // m_stream_table.touch( i);
        stream->count++;

        stream->hist_id = ++m_hist_id;

        return {true, i}; // A prefetch base address found.
    }

    MYLOG ("UpdateMonitorStream: Table Not Found");

    return {false, 0};
}


// Check and update entries with SS_TRAINING status in the stream table.
bool StreamPrefetcher::UpdateTrainingStream (IntPtr current_address)
{
    IntPtr miss_block_address = MaskLineOffset (current_address);

    MYLOG ("UpdateTrainingStream(%ld)::current_address = %08lx, miss_block_address = %08lx",
           m_stream_table.size(), miss_block_address, current_address);

    for (size_t i = 0; i < m_stream_table.size(); i++) {
        Stream* stream = m_stream_table[i];

        // MYLOG ("UpdateTrainingStream: target_address = %08lx, stream_table[%ld] status=%d, start=0x%08lx, window=0x%lx",
        //        miss_block_address, i, static_cast<int>(stream->status), stream->orig, m_effectiveTrainingWindow);

        if (stream->status != SS_TRAINING)
            continue;

        IntPtr window = m_effectiveTrainingWindow;
        const IntPtr& start = stream->orig;

        // Check a missed address is in a training window.
        bool ascending = false;
        if (is_in_window( miss_block_address, start, window, true)) {
            ascending = true;
            MYLOG ("UpdateTrainingStream: stream_table[%ld] is hit(ascending). block_address = %08lx, start = %08lx, window = %lx",
                   i, miss_block_address, start, window);
        } else if (is_in_window( miss_block_address, start, window, false)) {
            ascending = false;
            MYLOG ("UpdateTrainingStream: stream_table[%ld] is hit(descending). block_address = %08lx, start = %08lx, window = %lx",
                   i, miss_block_address, start, window);
        } else {
            continue;
        }

        // Decide a stream direction of a stream when
        // an access is a first access (stream->count == 0)
        // in a training mode.
        if (stream->count == 0) {
            stream->ascending = ascending;
        }

        stream->hist_id = ++m_hist_id;
        if (stream->ascending == ascending) {
            // Update the training count.
            stream->count++;
        } else{
            // Reset a counter and a direction flag.
            stream->ascending = ascending;
            stream->count = 0;
            continue;   // A prefetch process is continued.
        }

        // State transition to MONITOR status.
        if (stream->count >= m_training_threshold) {
            stream->status = SS_MONITOR;
            MYLOG ("UpdateTrainingStream::TRAIN->MON : current_address = %08lx, entry index = %ld", current_address, i);
        }

        // Training
        // m_stream_table.touch(i);
        //stream->addr = miss_block_address;

        return true; // A prefetch process is finished.
    }

    MYLOG ("UpdateTrainingStream: Table Not Found");

    return false;
}


bool StreamPrefetcher::isInEntryRegion (IntPtr current_address)
{
    IntPtr block_address = MaskLineOffset (current_address);

    MYLOG ("isInEntryRegion::current_address = %08lx", current_address);

    for (size_t i = 0; i < m_stream_table.size(); i++) {
        Stream* stream = m_stream_table[i];

        if (stream->status != SS_MONITOR)
            continue;

        if (stream->ascending &&
            block_address >= stream->orig && block_address < stream->addr) {
           MYLOG ("isInEntryRegion::Hit. entry index = %ld", i);
           return true;
        } else if (!stream->ascending &&
                   block_address <= stream->orig && block_address > stream->addr) {
           MYLOG ("isInEntryRegion::Hit. entry index = %ld", i);
           return true;
        }
    }

    MYLOG ("isInEntryRegion::Not Found");

    return false;
}

// Allocate a new entry in the stream table.
void StreamPrefetcher::AllocateStream (IntPtr current_address)
{
    IntPtr miss_block_address = MaskLineOffset(current_address);

    Stream *stream = new Stream;
    stream->addr = miss_block_address;
    stream->orig = miss_block_address;
    stream->status = SS_TRAINING;
    stream->count = 0;

    if (m_stream_table.size() >= m_stream_table_size) {

       auto it = m_stream_table.begin();
       size_t min_index = 0;
       size_t min_hist_id = (*it)->hist_id;
       for (size_t i = 0; it != m_stream_table.end(); i++, it++) {
          if ((*it)->hist_id < min_hist_id) {
             min_index = i;
             min_hist_id = (*it)->hist_id;
          }
       }

       m_stream_table.erase(m_stream_table.begin() + min_index);
    }
    m_stream_table.push_back(stream);
    // m_stream_table.touch( target);

    MYLOG ("AllocateStream::current_address = %08lx. allocated index = %ld", current_address, m_stream_table.size());
}
