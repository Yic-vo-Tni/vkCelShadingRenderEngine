//
// Created by lenovo on 10/31/2024.
//

#ifndef VKCELSHADINGRENDERER_MEMORY_H
#define VKCELSHADINGRENDERER_MEMORY_H

namespace rs {

    template <typename T>
    class MemoryManager {
    public:
        MemoryManager()
                : pool(sizeof(T))
        {}

        T* allocate(size_t num = 1) {
            return static_cast<T*>(pool.ordered_malloc(num));
        }

        void deallocate(T* ptr, size_t num = 1) {
            pool.ordered_free(ptr, num);
        }

    private:
        boost::pool<> pool;
    };

//    template<typename T>
//    class MemoryPool {
//    public:
//        explicit MemoryPool(size_t poolSize) {
//            pageSize = static_cast<size_t>(GetSystemPageSize());
//            //poolSizeInBytes = poolSize * sizeof(T);
//            poolSizeInBytes = poolSize;
//
//            pool = static_cast<T*>(VirtualAlloc(nullptr, poolSizeInBytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
//            if (!pool) {
//                throw std::bad_alloc();
//            }
//            this->poolSize = poolSize;
//            offset = 0;
//        }
//
//        ~MemoryPool() {
//            if (pool) {
//                VirtualFree(pool, 0, MEM_RELEASE);
//            }
//        }
//
//        auto allocate(size_t size) -> T* {
//            size_t totalSize = size * sizeof(T);
//            if (offset + totalSize > poolSizeInBytes) {
//                throw std::bad_alloc();
//            }
//
//            T* r = pool + offset;
//            offset += totalSize;
//            return r;
//        }
//
//        auto reset() -> void {
//            offset = 0;
//        }
//
//    private:
//        T* pool;
//        size_t poolSize;
//        size_t poolSizeInBytes;
//        size_t offset;
//        size_t pageSize;
//
//        size_t GetSystemPageSize() {
//            SYSTEM_INFO sysInfo;
//            GetSystemInfo(&sysInfo);
//            return sysInfo.dwPageSize;
//        }
//    };
//
//    template<typename T>
//    class MemoryChunk {
//    public:
//        MemoryChunk(MemoryPool<T>& pool, size_t size)
//                : memoryPool(pool), chunkSize(size), currentOffset(0) {
//            chunkData = memoryPool.allocate(size);
//        }
//
//        T* allocate(size_t size) {
//            if (currentOffset + size > chunkSize) {
//                throw std::bad_alloc();
//            }
//            T* ptr = chunkData + currentOffset;
//            currentOffset += size;
//            return ptr;
//        }
//
//        T* getData() const{
//            return chunkData;
//        }
//
//        size_t getSize() const{
//            return currentOffset;
//        }
//
//        auto iterate(const std::function<void(T&)>& callback){
//            for(size_t i = 0; i < currentOffset; ++i){
//                callback(chunkData[i]);
//            }
//        }
//
//        void reset() {
//            currentOffset = 0;
//        }
//
//    private:
//        MemoryPool<T>& memoryPool;
//        T* chunkData;
//        size_t chunkSize;
//        size_t currentOffset;
//    };
//
//
//    template<typename T>
//    class MemoryChunkManager {
//    public:
//        MemoryChunkManager(MemoryPool<T>& pool, size_t defaultChunkSize = 1024)
//                : memoryPool(pool), defaultChunkSize(defaultChunkSize) {}
//
//        MemoryChunk<T>& createNewChunk() {
//            return createNewChunk(defaultChunkSize);
//        }
//
//        MemoryChunk<T>& createNewChunk(size_t customSize) {
//            chunks.emplace_back(std::make_unique<MemoryChunk<T>>(memoryPool, customSize));
//            return *chunks.back();
//        }
//
//        MemoryChunk<T>* createNewChunkPtr(size_t customSize){
//            chunks.emplace_back(std::make_unique<MemoryChunk<T>>(memoryPool, customSize));
//            return chunks.back().get();
//        }
//
//        void resetAllChunks() {
//            for (auto& chunk : chunks) {
//                chunk->reset();
//            }
//        }
//
//    private:
//        MemoryPool<T>& memoryPool;
//        size_t defaultChunkSize;
//        vot::vector<std::unique_ptr<MemoryChunk<T>>> chunks;
//    };

} // rs

#endif //VKCELSHADINGRENDERER_MEMORY_H
