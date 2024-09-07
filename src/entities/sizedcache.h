#pragma once
#include <deque>
#include <dpp/cache.h>

namespace RR
{
    // a version of dpp::cache with a maximum size.
    // once this maximum size is surpassed, any excessive elements are queued for deletion.
    template<class T, size_t Size>
    class sized_cache
    {
    public:
        using map_type = std::unordered_map<dpp::snowflake, T*>;

        ~sized_cache()
        {
            std::unique_lock l(cache_mutex);
            cache_map.clear();
            entries_ordered.clear();
        }

        void store(T* object)
        {
            if (!object)
                return;

            std::unique_lock l(cache_mutex);
            if (auto existing = cache_map.find(object->id); existing == cache_map.end())
            {
                cache_map[object->id] = object;
                entries_ordered.push_back(object->id);
            }
            else if (object != existing->second)
            {
                std::lock_guard<std::mutex> delete_lock(dpp::deletion_mutex);
                dpp::deletion_queue[existing->second] = time(nullptr);
                cache_map[object->id] = object;
            }

            if (entries_ordered.size() > Size)
            {
                std::lock_guard<std::mutex> delete_lock(dpp::deletion_mutex);
                for (std::pair<dpp::snowflake, bool> res = dequeue_excessive(); res.second; res = dequeue_excessive())
                    remove_intrnl(res.first);
            }
        }

        void remove(T* object)
        {
            if (!object)
                return;

            std::unique_lock l(cache_mutex);
            std::lock_guard<std::mutex> delete_lock(dpp::deletion_mutex);
            remove_intrnl(object->id);
        }

        T* find(dpp::snowflake id)
        {
            std::shared_lock l(cache_mutex);
            if (auto existing = cache_map.find(id); existing != cache_map.end())
                return existing->second;
            return nullptr;
        }

        size_t count()
        {
            std::shared_lock l(cache_mutex);
            return cache_map.size();
        }

        std::shared_mutex& get_mutex() { return cache_mutex; }
        map_type& get_container() { return cache_map; }

        void rehash()
        {
            std::unique_lock l(cache_mutex);
            map_type n(cache_map.begin(), cache_map.end());
            cache_map = n;
        }

        size_t bytes()
        {
            std::shared_lock l(cache_mutex);
            return sizeof(*this) + (cache_map.bucket_count() * sizeof(size_t));
        }
    private:
        map_type cache_map;
        std::shared_mutex cache_mutex;
        std::deque<dpp::snowflake> entries_ordered;

        std::pair<dpp::snowflake, bool> dequeue_excessive()
        {
            if (entries_ordered.size() > Size)
            {
                auto out = std::make_pair(entries_ordered.front(), true);
                entries_ordered.pop_front();
                return out;
            }

            return std::make_pair(dpp::snowflake{}, false);
        }

        void remove_intrnl(dpp::snowflake id)
        {
            if (auto existing = cache_map.find(id); existing != cache_map.end())
            {
                dpp::deletion_queue[existing->second] = time(nullptr);
                cache_map.erase(existing);
            }
        }
    };
}
