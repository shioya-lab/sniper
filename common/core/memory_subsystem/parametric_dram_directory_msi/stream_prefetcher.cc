#include "stream_prefetcher.h"
#include "simulator.h"
#include "config.hpp"

#include <cstdlib>

const IntPtr PAGE_SIZE = 4096;
const IntPtr PAGE_MASK = ~(PAGE_SIZE-1);


StreamPrefetcher::StreamPrefetcher(String configName, core_id_t _core_id, UInt32 _shared_cores)
   : core_id(_core_id)
   , shared_cores(_shared_cores)
   , configName(configName)
   , m_stream_table_size     (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/stream/table_size", core_id))
   , m_distance              (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/stream/distance", core_id))
   , m_degree                (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/stream/degree", core_id))
   , m_training_window_size  (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/stream/training_window", core_id))
   , m_training_threshold    (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/stream/training_threshold", core_id))
   , m_cache_block_size      (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/cache_block_size", core_id))
   , m_enable_log            (Sim()->getCfg()->getBoolArray("log/enable_stream_prefetcher_log", core_id))
{
    if(phase == INIT_PRE_CONNECTION){
        // Setup remaining members from loaded parameters.
        m_effectiveDistance       = m_cache_block_size * m_distance;
        m_effectiveTrainingWindow = m_cache_block_size * m_training_window_size;
    }
}


// Returns whether the 'addr' is in a window specified
// by the remaining arguments or not. The 'ascending' means
// a direction of a window.
bool StreamPrefetcher::is_in_window( IntPtr addr, IntPtr start, IntPtr windowSize, bool ascending)
{
    IntPtr line_addr  = MaskLineOffset(addr);
    IntPtr line_start = MaskLineOffset(start);

    if (m_enable_log) {
        fprintf(stderr, "   %s: is_in_window::Trying to check : line_addr=%08lx, line_start=%08lx, window=%08lx, ascending=%d\n",
                        configName.c_str(), line_addr, line_start, windowSize, ascending);
    }
    if (ascending) {
        return line_start <= line_addr && line_addr < line_start + windowSize;
    } else{
        return line_start - windowSize <= line_addr && line_addr < line_start;
    }
}



// Masks offset bits of the cache line in the 'addr'.
IntPtr StreamPrefetcher::MaskLineOffset( IntPtr addr)
{
    return addr & ~((1 << m_cache_block_size) - 1);
}

std::vector<IntPtr>
StreamPrefetcher::getNextAddress(IntPtr current_address, IntPtr pc, core_id_t _core_id)
{
    if (m_enable_log) {
      fprintf(stderr, "  %s StreamPrefetcher::getNextAddress(%08lx) start :\n", configName.c_str(), current_address);
    }
    std::vector<IntPtr> addresses;

    auto monitor_ret = UpdateMonitorStream(current_address);
    if (monitor_ret.first) {
        size_t table_index = monitor_ret.second;
        auto stream = m_stream_table[table_index];

        IntPtr prefetch = current_address + (stream->ascending ? m_cache_block_size : -m_cache_block_size);
        for (int i = 0; i < m_degree; i++) {
          if (m_enable_log) {
            fprintf(stderr, "  %s StreamPrefetcher::getNextAddress() pushing address %08lx\n", configName.c_str(), prefetch);
          }
          addresses.push_back(prefetch);
          prefetch = prefetch + (stream->ascending ? m_cache_block_size : -m_cache_block_size);
        }
    }

    if (!UpdateTrainingStream(current_address)) {
        AllocateStream(current_address);
    }

    return addresses;
}


// Check and update entries with MONITOR status in the stream table.
// Returns whether any entries are updated or not.
std::pair<bool, size_t> StreamPrefetcher::UpdateMonitorStream(IntPtr current_address)
{
    if (m_enable_log) {
      fprintf(stderr, "   %s StreamPrefetcher::UpdateMonitorStream() start :\n", configName.c_str());
    }

    IntPtr miss_block_address = MaskLineOffset( current_address);

    // Monitor
    for (size_t i = 0; i < m_stream_table.size(); i++){
        Stream* stream = m_stream_table[i];
        if (stream->status != SS_MONITOR)
            continue;

        // Check a missed address is in a prefetch window.
        if (!is_in_window( miss_block_address, stream->addr, m_effectiveDistance, stream->ascending)) {
            continue;
        }

        if (m_enable_log) {
            fprintf(stderr, "   %s: UpdateMonitorStream::current_address hit = %08lx, entry index = %ld\n",
                    configName.c_str(),
                    current_address, i);
        }

        // Update the table
        // m_stream_table.touch( i);
        stream->count++;

        return {true, i}; // A prefetch base address found.
    }

    return {false, 0};
}


// Check and update entries with SS_TRAINING status in the stream table.
bool StreamPrefetcher::UpdateTrainingStream (IntPtr current_address)
{
    IntPtr miss_block_address = MaskLineOffset (current_address);

    if (m_enable_log) {
        fprintf(stderr, "   %s: UpdateTrainingStream(%ld)::current_address = %08lx, miss_block_address = %08lx\n",
                configName.c_str(), m_stream_table.size(), miss_block_address, current_address);
        fprintf (stderr, "   ");
        for (size_t i = 0; i < m_stream_table.size(); i++) {
            fprintf (stderr, "(%2ld,%s,%08lx)", i, m_stream_table[i]->status == SS_TRAINING ? "T" :
                              m_stream_table[i]->status == SS_MONITOR ? "M" : "X",
                              m_stream_table[i]->addr);
            if (i % 8 == 8-1) {
                fprintf(stderr, "\n");
            }
        }
        fprintf(stderr, "\n");
    }

    for (size_t i = 0; i < m_stream_table.size(); i++) {
        Stream* stream = m_stream_table[i];
        if (stream->status != SS_TRAINING)
            continue;

        IntPtr window = m_effectiveTrainingWindow;
        const IntPtr& start = stream->orig;

        // Check a missed address is in a training window.
        bool inWindow  = false;
        bool ascending = false;
        if (is_in_window( miss_block_address, start, window, true)) {
            inWindow  = true;
            ascending = true;
            if (m_enable_log) {
              fprintf(stderr, "   %s: UpdateTraining: stream_table[%ld] is hit(ascending)\n", configName.c_str(), i);
            }
        } else if (is_in_window( miss_block_address, start, window, false)) {
            inWindow  = true;
            ascending = false;
            if (m_enable_log) {
                fprintf(stderr, "   %s: UpdateTraining: stream_table[%ld] is hit(descending)\n", configName.c_str(), i);
            }
        } else {
            continue;
        }

        // Decide a stream direction of a stream when
        // an access is a first access (stream->count == 0)
        // in a training mode.
        if (stream->count == 0) {
            stream->ascending = ascending;
        }

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
            if (m_enable_log) {
                fprintf(stderr, "   %s: UpdateTrainingStream::TRAIN->MON : current_address = %08lx, entry index = %ld\n",
                        configName.c_str(),
                        current_address, i);
            }
        }

        // Training
        // m_stream_table.touch(i);
        //stream->addr = miss_block_address;

        return true; // A prefetch process is finished.
    }

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
        // m_stream_table.pop_front();
        m_stream_table.erase(m_stream_table.begin());
    }
    m_stream_table.push_back(stream);
    // m_stream_table.touch( target);

    if (m_enable_log) {
        fprintf(stderr, " %s: AllocateStream::current_address = %08lx\n",
                configName.c_str(), current_address);
    }
}
