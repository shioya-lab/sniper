#ifndef INDIRECT_MEMORY_PREFETCHER_H
#define INDIRECT_MEMORY_PREFETCHER_H

#define TRACING_ON 1
#include "mem/cache/prefetch/indirect_memory.hh"
#include "mem/cache/tags/indexing_policies/set_associative.hh"
#include "mem/cache/replacement_policies/lru_rp.hh"
#include "params/IndirectMemoryPrefetcher.hh"
#include "params/LRURP.hh"
#include "params/SetAssociative.hh"
#include "prefetcher.h"
#include "sim/system.hh"
#include "sim/workload.hh"

class IndirectMemoryPrefetcher final : public Prefetcher
{
    public:
        IndirectMemoryPrefetcher(String configName, core_id_t core_id)
            : m_clk_domain((gem5::ClockDomainParams) {}, nullptr)
            , m_workload((gem5::StubWorkloadParams) {})
            , m_sys(sysParams(m_workload))
            , m_ipd_table_indexing_policy(ipdTableIndexingPolicyParams(configName, core_id))
            , m_ipd_table_replacement_policy((gem5::LRURPParams) {})
            , m_pt_table_indexing_policy(ptTableIndexingPolicyParams(configName, core_id))
            , m_pt_table_replacement_policy((gem5::LRURPParams) {})
            , m_prefetch(params(configName, core_id, m_clk_domain, m_sys,
                                m_ipd_table_indexing_policy, m_ipd_table_replacement_policy,
                                m_pt_table_indexing_policy, m_pt_table_replacement_policy))
        {
        }

        virtual std::vector<IntPtr> getNextAddress(IntPtr current_address, IntPtr eip, core_id_t core_id) override
        {
#if 0
            Request req(current_address);
            MemCmd cmd;
            Packet packet(req, cmd);
            std::vector<AddrPriority> addr_priorities;
            CacheAccessor cache;

            m_prefetch.calculatePrefetch(&packet, addr_priorities, cache);

            std::vector<IntPtr> addresses(addr_priorities.size());
            for (size_t index = 0; index < addr_priorities.size(); index++)
            {
                addresses[index] = addr_priorities[index].first;
            }

            return addresses;
#else
            return {};
#endif
        }

    private:
        static gem5::SystemParams sysParams(gem5::Workload& workload)
        {
            gem5::SystemParams params = {};

            params.cache_line_size = 16;
            params.workload = &workload;

            return params;
        }

        static gem5::SetAssociativeParams ptTableIndexingPolicyParams(String configName, core_id_t core_id)
        {
            gem5::SetAssociativeParams params = {};
            auto& cfg = *Sim()->getCfg();

            params.assoc = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/pt_table_assoc", core_id);
            params.entry_size = 1;
            params.size = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/pt_table_entries", core_id);

            return params;
        }

        static gem5::SetAssociativeParams ipdTableIndexingPolicyParams(String configName, core_id_t core_id)
        {
            gem5::SetAssociativeParams params = {};
            auto& cfg = *Sim()->getCfg();

            params.assoc = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/ipd_table_assoc", core_id);
            params.entry_size = 1;
            params.size = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/ipd_table_entries", core_id);

            return params;
        }

        static gem5::IndirectMemoryPrefetcherParams params(String configName, core_id_t core_id,
                                                           gem5::ClockDomain& clk_domain, gem5::System& sys,
                                                           gem5::BaseIndexingPolicy& ipd_table_indexing_policy,
                                                           gem5::replacement_policy::Base& ipd_table_replacement_policy,
                                                           gem5::BaseIndexingPolicy& pt_table_indexing_policy,
                                                           gem5::replacement_policy::Base& pt_table_replacement_policy)
        {
            gem5::IndirectMemoryPrefetcherParams params = {};
            auto& cfg = *Sim()->getCfg();

            params.name = "IndirectMemoryPrefetcher";
            params.clk_domain = &clk_domain;
            params.block_size = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/block_size", core_id);
            params.on_data = cfg.getBoolArray("perf_model/" + configName + "/prefetcher/indirect_memory/on_data", core_id);
            params.on_inst = cfg.getBoolArray("perf_model/" + configName + "/prefetcher/indirect_memory/on_inst", core_id);
            params.on_miss = cfg.getBoolArray("perf_model/" + configName + "/prefetcher/indirect_memory/on_miss", core_id);
            params.on_read = cfg.getBoolArray("perf_model/" + configName + "/prefetcher/indirect_memory/on_read", core_id);
            params.on_write = cfg.getBoolArray("perf_model/" + configName + "/prefetcher/indirect_memory/on_write", core_id);
            params.page_bytes = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/page_bytes", core_id);
            params.prefetch_on_access = cfg.getBoolArray("perf_model/" + configName + "/prefetcher/indirect_memory/prefetch_on_access", core_id);
            params.prefetch_on_pf_hit = cfg.getBoolArray("perf_model/" + configName + "/prefetcher/indirect_memory/prefetch_on_pf_hit", core_id);
            params.sys = &sys;
            params.use_virtual_addresses = cfg.getBoolArray("perf_model/" + configName + "/prefetcher/indirect_memory/use_virtual_addresses", core_id);
            params.cache_snoop = cfg.getBoolArray("perf_model/" + configName + "/prefetcher/indirect_memory/cache_snoop", core_id);
            params.latency = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/latency", core_id);
            params.max_prefetch_requests_with_pending_translation = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/max_prefetch_requests_with_pending_translation", core_id);
            params.queue_filter = cfg.getBoolArray("perf_model/" + configName + "/prefetcher/indirect_memory/queue_filter", core_id);
            params.queue_size = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/queue_size", core_id);
            params.queue_squash = cfg.getBoolArray("perf_model/" + configName + "/prefetcher/indirect_memory/queue_squash", core_id);
            params.tag_prefetch = cfg.getBoolArray("perf_model/" + configName + "/prefetcher/indirect_memory/tag_prefetch", core_id);
            params.throttle_control_percentage = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/throttle_control_percentage", core_id);
            params.addr_array_len = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/addr_array_len", core_id);
            params.ipd_table_assoc = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/ipd_table_assoc", core_id);
            params.ipd_table_indexing_policy = &ipd_table_indexing_policy;
            params.ipd_table_replacement_policy = &ipd_table_replacement_policy;
            params.ipd_table_entries = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/ipd_table_entries", core_id);
            params.max_prefetch_distance = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/max_prefetch_distance", core_id);
            params.num_indirect_counter_bits = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/num_indirect_counter_bits", core_id);
            params.prefetch_threshold = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/prefetch_threshold", core_id);
            params.pt_table_assoc = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/pt_table_assoc", core_id);
            params.pt_table_entries = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/pt_table_entries", core_id);
            params.pt_table_indexing_policy = &pt_table_indexing_policy;
            params.pt_table_replacement_policy = &pt_table_replacement_policy;
            params.shift_values = {2, 3, 4, -3};
            params.stream_counter_threshold = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/stream_counter_threshold", core_id);
            params.streaming_distance = cfg.getIntArray("perf_model/" + configName + "/prefetcher/indirect_memory/streaming_distance", core_id);

            return params;
        }

        gem5::ClockDomain m_clk_domain;
        gem5::StubWorkload m_workload;
        gem5::System m_sys;
        gem5::SetAssociative m_ipd_table_indexing_policy;
        gem5::replacement_policy::LRU m_ipd_table_replacement_policy;
        gem5::SetAssociative m_pt_table_indexing_policy;
        gem5::replacement_policy::LRU m_pt_table_replacement_policy;
        gem5::prefetch::IndirectMemory m_prefetch;
};

#endif
