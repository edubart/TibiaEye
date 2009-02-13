#ifndef SINGLETON_H
#define SINGLETON_H

template <class T>
class Singleton
{
public:
	static T *instance() {
        return mInstance;
    }

protected:
    Singleton() {
        mInstance = (T*)(this);
    }

    virtual ~Singleton() {
        mInstance = 0;
    }

private:
    static T *mInstance;
};

template <class T>
T *Singleton<T>::mInstance = 0;

#endif // SINGLETON_H
