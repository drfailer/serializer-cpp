#include <serializer/serialize.hpp>

/******************************************************************************/
/*                                  block id                                  */
/******************************************************************************/

enum BlockId {
    Input,
};

/******************************************************************************/
/*                                 type table                                 */
/******************************************************************************/

template <typename T> class Matrix;
template <typename T> struct PartialSum;
template <typename T, BlockId Id> class MatrixBlock;
template <typename T>
using TypeTable = serializer::tools::IdTable<char, Matrix<T>, PartialSum<T>,
                                             MatrixBlock<T, Input>>;

/******************************************************************************/
/*                          matrix and matrix blocks                          */
/******************************************************************************/

template <typename T> class Matrix {
  public:
    Matrix() = default;
    Matrix(size_t width, size_t height, size_t blockSize, T *data)
        : width_(width), height_(height), blockSize_(blockSize),
          nbRawBlocks_(height / blockSize + (height % blockSize == 0 ? 0 : 1)),
          nbColBlocks_(width / blockSize + (width % blockSize == 0 ? 0 : 1)),
          data_(data) {}

    SERIALIZE(serializer::tools::getId<Matrix<T>>(TypeTable<T>()), width_,
              height_, blockSize_, nbRawBlocks_, nbColBlocks_,
              SER_DARR(data_, width_, height_));

    size_t width() const { return width_; }
    size_t height() const { return height_; }
    size_t blockSize() const { return blockSize_; }
    size_t nbRawBlocks() const { return nbRawBlocks_; }
    size_t nbColBlocks() const { return nbColBlocks_; }
    T *data() { return data_; }
    void data(T *data) { data_ = data; }

  private:
    size_t width_ = 0;
    size_t height_ = 0;
    size_t blockSize_ = 0;
    size_t nbRawBlocks_ = 0;
    size_t nbColBlocks_ = 0;
    T *data_ = nullptr;
};

template <typename T, BlockId Id> class MatrixBlock {
  public:
    MatrixBlock() = default;
    MatrixBlock(size_t x, size_t y, size_t matrixWidth, size_t matrixHeight,
                size_t blockSize, size_t dataSize, T *data)
        : x_(x), y_(y), matrixWidth_(matrixWidth), matrixHeight_(matrixHeight),
          blockSize_(blockSize), dataSize_(dataSize), data_(data) {}

    SERIALIZE(serializer::tools::getId<MatrixBlock<T, Id>>(TypeTable<T>()), x_,
              y_, matrixWidth_, matrixHeight_, blockSize_,
              dataSize_, SER_DARR(data_, dataSize_));

    size_t x() const { return x_; }
    size_t y() const { return y_; }
    size_t matrixWidth() const { return matrixWidth_; }
    size_t matrixHeight() const { return matrixHeight_; }
    size_t blockSize() const { return blockSize_; }
    T *data() { return data_; }
    void data(T *data) { data_ = data; }

  private:
    size_t x_ = 0;
    size_t y_ = 0;
    size_t matrixWidth_;
    size_t matrixHeight_;
    size_t blockSize_ = 0;
    size_t dataSize_ = 0;
    T *data_ = nullptr;
};

template <typename T> struct PartialSum {
    SERIALIZE(serializer::tools::getId<PartialSum<T>>(TypeTable<T>()), value);
    T value = 0;
};

/******************************************************************************/
/*                                  network                                   */
/******************************************************************************/

struct Network {
    static inline serializer::default_mem_type data;
    static void send(serializer::default_mem_type mem) {
        data.append(data.size(), mem.data(), mem.size());
    }
    static serializer::default_mem_type rcv() {
        serializer::default_mem_type result = data;
        data.clear();
        return result;
    }
};

/******************************************************************************/
/*                              abstract classes                              */
/******************************************************************************/

template <typename... Inputs> struct In {};

template <typename... Outputs> struct Out {};

template <typename T> struct Execute;
template <typename T> struct Executes;
template <typename T> struct Send;
template <typename T> struct Sends;

template <typename... Types>
struct Executes<In<Types...>> : Execute<Types>... {};

template <typename... Types> struct Sends<Out<Types...>> : Send<Types>... {};

template <typename Input, typename Output>
struct Task : Executes<Input>, Sends<Output> {};

template <typename T> struct Execute {
    virtual void execute(std::shared_ptr<T>) = 0;
};

template <typename T> struct Send {
    serializer::default_mem_type mem;

    void send(std::shared_ptr<T> elt) {
        elt->serialize(mem);
        Network::send(mem);
    }
};

template <typename... Tasks> struct RunExecute {
    RunExecute(std::tuple<std::shared_ptr<Tasks>...> tasks) : tasks(tasks) {}

    template <typename T, typename Task>
    void runExecute(std::shared_ptr<T> ptr, std::shared_ptr<Task> task) {
        if constexpr (requires { task->execute(ptr); }) {
            task->execute(ptr);
        }
    }

    template <typename T, size_t... Idx>
    void runExecute(std::shared_ptr<T> ptr, std::index_sequence<Idx...>) {
        ([&] { runExecute(ptr, std::get<Idx>(tasks)); }(), ...);
    }

    template <typename T> void runExecute(std::shared_ptr<T> ptr) {
        runExecute(ptr, std::make_index_sequence<sizeof...(Tasks)>());
    }

    std::tuple<std::shared_ptr<Tasks>...> tasks;
};

/******************************************************************************/
/*                                task manager                                */
/******************************************************************************/

template <typename TypeTable, typename... Tasks>
struct TaskManager : RunExecute<Tasks...> {
    TaskManager(std::shared_ptr<Tasks>... tasks)
        : RunExecute<Tasks...>(std::make_tuple(tasks...)) {}

    void receive(serializer::default_mem_type buff) {
        size_t pos = 0;

        while (pos < buff.size()) {
            auto id = serializer::deserialize_id<typename TypeTable::id_type>(
                buff, pos);
            serializer::tools::applyId(id, TypeTable(), [&]<typename T>() {
                auto v = std::make_shared<T>();
                pos = v->deserialize(buff, pos);
                this->runExecute(v);
            });
        }
    }
};

/******************************************************************************/
/*                                   tasks                                    */
/******************************************************************************/

template <typename T>
struct SplitTask : Task<In<Matrix<T>>, Out<MatrixBlock<T, Input>>> {
    void execute(std::shared_ptr<Matrix<T>> matrix) override {
        for (size_t i = 0; i < matrix->nbRawBlocks(); ++i) {
            for (size_t j = 0; j < matrix->nbColBlocks(); ++j) {
                auto block = std::make_shared<MatrixBlock<T, Input>>(
                    i * matrix->blockSize(), j * matrix->blockSize(),
                    matrix->width(), matrix->height(), matrix->blockSize(),
                    matrix->height() * matrix->width(), matrix->data());
                this->send(block);
            }
        }

        delete[] matrix->data();
        matrix->data(nullptr);
    }
};

template <typename T>
struct ComputeTask : Task<In<MatrixBlock<T, Input>>, Out<PartialSum<T>>> {
    void execute(std::shared_ptr<MatrixBlock<T, Input>> block) override {
        size_t jend = std::min(block->matrixWidth(), block->x() + block->blockSize());
        size_t iend = std::min(block->matrixHeight(), block->y() + block->blockSize());
        T result = 0;

        for (size_t i = block->y(); i < iend; ++i) {
            for (size_t j = block->x(); j < jend; ++j) {
                result += block->data()[i * block->matrixWidth() + j];
            }
        }
        this->send(std::make_shared<PartialSum<T>>(result));

        // delete the block here since the memory has been allocated during the
        // deserialzation
        delete[] block->data();
        block->data(nullptr);
    }
};

template <typename T> struct ResultTask : Task<In<PartialSum<T>>, Out<>> {
    void execute(std::shared_ptr<PartialSum<T>> ps) override {
        result += ps->value;
    }
    size_t result = 0;
};
