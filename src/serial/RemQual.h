#pragma once

//
// RemQual
// Strips down qualified types/references/pointers to a single unqualified type, for passing into
// a templated type as a typename parameter.
//

template <class T>
struct RemQual
{
    typedef T type;
};

template <class T>
struct RemQual<const T>
{
	typedef typename RemQual<T>::type type;
};

template <class T>
struct RemQual<T&>
{
	typedef typename RemQual<T>::type type;
};

template <class T>
struct RemQual<const T&>
{
	typedef typename RemQual<T>::type type;
};

template <class T>
struct RemQual<T&&>
{
	typedef typename RemQual<T>::type type;
};

template <class T>
struct RemQual<const T *>
{
	typedef typename RemQual<T *>::type type;
};