/*
Source: https://jguegant.github.io/blogs/tech/thread-safe-multi-type-map.html
Creative Commons Attribution-ShareAlike 4.0 International License
Original source code has been altered.
*/

//
// Created by jguegant on 1/30/16.
//

#ifndef NFE_REPOSITORY_HPP
#define NFE_REPOSITORY_HPP

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>
#include <functional>

namespace NFE
{
    struct DefaultRepositorySlotKey;

    template <class Type, class Key = DefaultRepositorySlotKey> class RepositorySlot;

    template <class Type, class Key>
    class Watcher
    {
        public:
            Watcher(RepositorySlot<Type, Key>& RepositorySlot):
                    RepositorySlot_(RepositorySlot),
                    hasBeenChanged_(false)
            {
            }

            Watcher(const Watcher&) = delete;

            Watcher & operator=(const Watcher&) = delete;

            bool hasBeenChanged() const
            {
                return hasBeenChanged_;
            }

            void triggerChanges()
            {
                hasBeenChanged_ = true;
            }

            auto get() -> decltype(std::declval<RepositorySlot<Type, Key>>().doGet())
            {
                hasBeenChanged_ = false; // Note: even if there is an update of the value between this line and the getValue one,
                // we will still have the latest version.
                // Note 2: atomic_bool automatically use a barrier and the two operations can't be inversed.
                return RepositorySlot_.doGet();
            }

        private:
            RepositorySlot<Type, Key>& RepositorySlot_;
            std::atomic_bool hasBeenChanged_;
    };


    template <class Type, class Key = DefaultRepositorySlotKey>
    using WatcherPtr = std::unique_ptr<Watcher<Type, Key>,
    std::function<void(Watcher<Type, Key>*)>>;

    template <class Type, class Key>
    class RepositorySlot
    {
        public:
            using ThisType = RepositorySlot<Type, Key>;
            using WatcherType = Watcher<Type, Key>;
            using WatcherTypePtr = std::unique_ptr<WatcherType, std::function<void(WatcherType*)>>;

        public:
            std::shared_ptr<Type> doGet() const
            {
                return std::atomic_load(&value_);
            }

            void doSet(const std::shared_ptr<Type> &value)
            {
                std::atomic_exchange(&value_, value);
                signal();
            }

            WatcherTypePtr doGetWatcher()
            {
                WatcherTypePtr watcher(new WatcherType(*this), [this](WatcherType* toBeDelete) {
                    this->unregisterWatcher(toBeDelete);});

                registerWatcher(watcher.get());

                return watcher;
            }

        private:
            void registerWatcher(WatcherType* newWatcher)
            {
                std::lock_guard<std::mutex> l(watchers_mutex_);
                watchers_.push_back(newWatcher);
            }

            void unregisterWatcher(WatcherType *toBeDelete)
            {
                std::lock_guard<std::mutex> l(watchers_mutex_);
                watchers_.erase(std::remove(watchers_.begin(), watchers_.end(), toBeDelete), watchers_.end());

                delete toBeDelete;
            }

            void signal()
            {
                std::lock_guard<std::mutex> l(watchers_mutex_);
                for (auto watcher : watchers_)
                {
                    watcher->triggerChanges();
                }
            }

        private:
            std::vector<WatcherType*> watchers_;
            std::shared_ptr<Type> value_;
            std::mutex watchers_mutex_;
    };

    template<class... RepositorySlots>
    class Repository : private RepositorySlots...
    {
        public:
            template <class Type, class Key = DefaultRepositorySlotKey>
            std::shared_ptr<Type> get()
            {
                static_assert(std::is_base_of<RepositorySlot<Type, Key>, Repository<RepositorySlots...>>::value,
                              "Please ensure that this type or this key exists in this repository");
                return RepositorySlot<Type, Key>::doGet();
            }

            template <class Type, class Key = DefaultRepositorySlotKey>
            void set(const std::shared_ptr<Type>& value)
            {
                static_assert(std::is_base_of<RepositorySlot<Type, Key>, Repository<RepositorySlots...>>::value,
                              "Please ensure that this type or this key exists in this repository");
                RepositorySlot<Type, Key>::doSet(value);
            }

            template <class Type, class Key = DefaultRepositorySlotKey, class ...Args>
            void emplace(Args&&... args)
            {
                static_assert(std::is_base_of<RepositorySlot<Type, Key>, Repository<RepositorySlots...>>::value,
                              "Please ensure that this type or this key exists in this repository");
                RepositorySlot<Type, Key>::doSet(std::make_shared<Type>(std::forward<Args>(args)...));
            }

            template <class Type, class Key = DefaultRepositorySlotKey>
            auto getWatcher()
            {
                static_assert(std::is_base_of<RepositorySlot<Type, Key>, Repository<RepositorySlots...>>::value,
                              "Please ensure that this type or this key exists in this repository");
                return RepositorySlot<Type, Key>::doGetWatcher();
            }
    };

}

#endif // NFE_REPOSITORY_HPP

