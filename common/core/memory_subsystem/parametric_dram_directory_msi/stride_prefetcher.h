#ifndef STRIDE_PREFETCHER_H
#define STRIDE_PREFETCHER_H

#define TRACING_ON 1
#include "mem/cache/prefetch/stride.hh"
#include "mem/cache/replacement_policies/random_rp.hh"
#include "mem/cache/tags/indexing_policies/set_associative.hh"
#include "params/RandomRP.hh"
#include "params/SetAssociative.hh"
#include "params/StridePrefetcher.hh"
#include "prefetcher.h"
#include "sim/system.hh"
#include "sim/workload.hh"

class StridePrefetcher final : public Prefetcher
{
    public:
        StridePrefetcher(String configName, core_id_t core_id)
            : m_clk_domain(clkDomainParams(), nullptr)
            , m_workload(workloadParams())
            , m_sys(sysParams(configName, core_id, m_workload))
            , m_table_indexing_policy(tableIndexingPolicyParams(configName, core_id))
            , m_table_replacement_policy(tableReplacementPolicyParams())
            , m_prefetch(params(configName, core_id, m_clk_domain, m_sys,
                                m_table_indexing_policy, m_table_replacement_policy))
        {
        }

        virtual std::vector<IntPtr> getNextAddress(IntPtr current_address, IntPtr eip, core_id_t core_id) override
        {
            static const uint8_t data = 0;
            gem5::Gem5Internal::_curTickPtr = &m_tick;
            auto req = std::make_shared<gem5::Request>(current_address, 1, 0, core_id, eip, gem5::InvalidContextID);
            req->setPaddr(current_address);
            gem5::Packet pkt(req, gem5::MemCmd());
            pkt.dataStaticConst(&data);
            gem5::prefetch::Stride::PrefetchInfo pfi(&pkt, current_address, false);
            std::vector<gem5::prefetch::Stride::AddrPriority> addr_priorities;

            m_prefetch.calculatePrefetch(pfi, addr_priorities, CacheAccessor());

            std::vector<IntPtr> addresses(addr_priorities.size());
            for (size_t index = 0; index < addr_priorities.size(); index++)
            {
                addresses[index] = addr_priorities[index].first;
            }

            return addresses;
        }

    private:
        struct CacheAccessor final : gem5::CacheAccessor
        {
            virtual bool inCache(gem5::Addr addr, bool is_secure) const override
            {
                throw std::logic_error("not implemented");
            }

            virtual bool hasBeenPrefetched(gem5::Addr addr, bool is_secure) const override
            {
                throw std::logic_error("not implemented");
            }

            virtual bool hasBeenPrefetched(gem5::Addr addr, bool is_secure,
                                           gem5::RequestorID requestor) const override
            {
                throw std::logic_error("not implemented");
            }

            virtual bool inMissQueue(gem5::Addr addr, bool is_secure) const override
            {
                throw std::logic_error("not implemented");
            }

            virtual bool coalesce() const override
            {
                throw std::logic_error("not implemented");
            }
        };

        static gem5::ClockDomainParams clkDomainParams()
        {
            gem5::ClockDomainParams params;

            params.eventq_index = 0;

            return params;
        }

        static gem5::StubWorkloadParams workloadParams()
        {
            gem5::StubWorkloadParams params;

            params.wait_for_remote_gdb = false;
            params.eventq_index = 0;
            params.entry = 0;
            params.byte_order = gem5::ByteOrder::little;

            return params;
        }

        static gem5::SystemParams sysParams(String configName, core_id_t core_id,
                                            gem5::Workload& workload)
        {
            gem5::SystemParams params;
            auto& cfg = *Sim()->getCfg();

            params.eventq_index = 0;
            params.mem_mode = gem5::enums::atomic;
            params.thermal_model = nullptr;
            params.mmap_using_noreserve = false;
            params.auto_unlink_shared_backstore = false;
            params.cache_line_size = cfg.getIntArray("perf_model/" + configName + "/cache_block_size", core_id);
            params.exit_on_work_items = false;
            params.work_item_id = -1;
            params.num_work_ids = 16;
            params.work_begin_cpu_id_exit = -1;
            params.work_begin_ckpt_count = 0;
            params.work_begin_exit_count = 0;
            params.work_end_ckpt_count = 0;
            params.work_end_exit_count = 0;
            params.work_cpus_ckpt_count = 0;
            params.workload = &workload;
            params.init_param = 0;
            params.multi_thread = false;
            params.m5ops_base = 0;

            return params;
        }

        static gem5::SetAssociativeParams tableIndexingPolicyParams(String configName, core_id_t core_id)
        {
            gem5::SetAssociativeParams params = {};
            auto& cfg = *Sim()->getCfg();

            params.eventq_index = 0;
            params.assoc = cfg.getIntArray("perf_model/" + configName + "/prefetcher/stride/table_assoc", core_id);
            params.entry_size = 1;
            params.size = cfg.getIntArray("perf_model/" + configName + "/prefetcher/stride/table_entries", core_id);

            return params;
        }

        static gem5::RandomRPParams tableReplacementPolicyParams()
        {
            gem5::RandomRPParams params;

            params.eventq_index = 0;

            return params;
        }

        static gem5::StridePrefetcherParams params(String configName, core_id_t core_id,
                                                   gem5::ClockDomain& clk_domain, gem5::System& sys,
                                                   gem5::BaseIndexingPolicy& table_indexing_policy,
                                                   gem5::replacement_policy::Base& table_replacement_policy)
        {
            gem5::StridePrefetcherParams params = {};
            auto& cfg = *Sim()->getCfg();

            params.name = "StridePrefetcher";
            params.eventq_index = 0;
            params.clk_domain = &clk_domain;
            params.power_state = nullptr;
            params.sys = &sys;
            params.block_size = sys.cacheLineSize();
            params.on_miss = false;
            params.on_read = true;
            params.on_write = true;
            params.on_data = true;
            params.on_inst = true;
            params.prefetch_on_access = false;
            params.prefetch_on_pf_hit = false;
            params.use_virtual_addresses = false;
            params.page_bytes = 4096;
            params.latency = 1;
            params.queue_size = 32;
            params.max_prefetch_requests_with_pending_translation = 32;
            params.queue_squash = true;
            params.queue_filter = true;
            params.cache_snoop = false;
            params.tag_prefetch = true;
            params.throttle_control_percentage = 0;
            params.confidence_counter_bits = cfg.getIntArray("perf_model/" + configName + "/prefetcher/stride/confidence_counter_bits", core_id);
            params.confidence_threshold = cfg.getIntArray("perf_model/" + configName + "/prefetcher/stride/confidence_threshold", core_id);
            params.degree = cfg.getIntArray("perf_model/" + configName + "/prefetcher/stride/degree", core_id);
            params.initial_confidence = cfg.getIntArray("perf_model/" + configName + "/prefetcher/stride/initial_confidence", core_id);
            params.table_assoc = cfg.getIntArray("perf_model/" + configName + "/prefetcher/stride/table_assoc", core_id);
            params.table_entries = cfg.getIntArray("perf_model/" + configName + "/prefetcher/stride/table_entries", core_id);
            params.table_indexing_policy = &table_indexing_policy;
            params.table_replacement_policy = &table_replacement_policy;
            params.use_requestor_id = cfg.getBoolArray("perf_model/" + configName + "/prefetcher/stride/use_requestor_id", core_id);

            return params;
        }

        gem5::ClockDomain m_clk_domain;
        gem5::StubWorkload m_workload;
        gem5::System m_sys;
        gem5::SetAssociative m_table_indexing_policy;
        gem5::replacement_policy::Random m_table_replacement_policy;
        gem5::prefetch::Stride m_prefetch;
        gem5::Tick m_tick;
};

#endif
