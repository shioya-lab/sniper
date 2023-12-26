#ifndef __STREAM_PREFETCHER_H
#define __STREAM_PREFETCHER_H

#include "core.h"
#include "prefetcher.h"

class StreamPrefetcher : public Prefetcher
{
    public:
        StreamPrefetcher(String configName, core_id_t core_id, UInt32 shared_cores);
        virtual std::vector<IntPtr> getNextAddress(IntPtr current_address, Core::mem_op_t mem_op_type, IntPtr pc, uint64_t uop_idx, core_id_t core_id);

    private:
        // Initializing phase
        enum InitPhase
        {
            // After constructing and before object connection.
            // ParamExchange::LoadParam() must be called in this phase or later.
            INIT_PRE_CONNECTION,
            // After connection
            INIT_POST_CONNECTION
        };

        enum StreamStatus
        {
            SS_INVALID,
            SS_TRAINING,    // Including 'Allocated' state as 'trainingCount' == 0.
            SS_MONITOR
        };

        struct Stream
        {
            StreamStatus status;    // Stream status

            IntPtr orig;      // An first accessed address of a stream
            IntPtr addr;      // A current 'start' address of a stream

            int  count;     // Access count
            bool ascending; // Stream direction

            Stream() {
                Reset();
            }

            void Reset()
            {
                status = SS_INVALID;
                ascending = true;
                count = 0;
            }

            String ToString() const;
        };


        const core_id_t core_id;
        const UInt32    shared_cores;
        const String    configName;

        InitPhase phase;
        typedef std::vector< Stream *> StreamTable;

        StreamTable m_stream_table;

        int m_cache_block_size;
        int m_distance;
        int m_degree;
        size_t m_stream_table_size;
        int m_training_window_size;
        int m_training_threshold;

        const bool m_enable_log;

        uint64_t m_effectiveDistance;        // An effective distance is calculated
                                             // by a 'distance' parameter and a line size.
        uint64_t m_effectiveTrainingWindow;  //

        bool is_in_window( IntPtr addr, IntPtr start, IntPtr windowSize, bool ascending );

        IntPtr MaskLineOffset (IntPtr addr);

        typedef uint32_t CacheAccess;

        std::pair<bool, size_t> UpdateMonitorStream (const IntPtr address);
        bool UpdateTrainingStream(const IntPtr address);
        void AllocateStream(const IntPtr address);

};

#endif // __STREAM_PREFETCHER_H
