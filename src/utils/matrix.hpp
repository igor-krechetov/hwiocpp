#ifndef __UTILS_MATRIX_HPP__
#define __UTILS_MATRIX_HPP__

#include <list>
#include <vector>

template <typename T>
class matrix
{
public:
    using value_type = T;
    using MatrixDiff_t = std::list<std::pair<size_t, size_t>>;// <x, y>

public:
    matrix() : matrix(0) {}

    explicit matrix(const std::size_t n) : matrix(n, n, T()) {}

    matrix(const std::size_t w, const std::size_t h, const T defaultValue)
        : mWidth(w)
        , mHeight(h)
        , mData(w * h, defaultValue)
    {}

    // value_type& operator()(const size_t w, const size_t h)
    // {
    //     return mData.at(h * mWidth + w);
    // }
    inline std::size_t width() const
    {
        return mWidth;
    }

    inline std::size_t height() const
    {
        return mHeight;
    }

    const value_type& operator()(const size_t x, const size_t y) const
    {
        const value_type& val = mData.at(y * mWidth + x);
        return val;
    }

    void set(const size_t x, const size_t y, const T value)
    {
        mData[y * mWidth + x] = value;
    }

    void setRowValue(const size_t y, const T value)
    {
        if (y < mHeight)
        {
            for (size_t c = 0 ; c < mWidth; ++c)
            {
                mData[y * mWidth + c] = value;
            }
        }
    }

    void setColumnValue(const size_t x, const T value)
    {
        if (x < mWidth)
        {
            for (size_t r = 0 ; r < mHeight; ++r)
            {
                mData[r * mWidth + x] = value;
            }
        }
    }

    MatrixDiff_t compare(const matrix<T>& right)
    {
        MatrixDiff_t diff;

        if ((mWidth == right.mWidth) && (mHeight == right.mHeight))
        {
            for (size_t i = 0 ; i < mData.size(); ++i)
            {
                if (right.mData[i] != mData[i])
                {
                    diff.push_back(std::make_pair(i % mWidth, i / mWidth));
                }
            }
        }

        return diff;
    }

private:
    std::size_t mWidth = 0;
    std::size_t mHeight = 0;
    std::vector<T> mData;
};

#endif // __UTILS_MATRIX_HPP__