#ifndef SINGLETON_H__
#define SINGLETON_H__

#define DEF_SINGLETON(NAME)     \
private:                        \
   static NAME _instance;       \
public:                         \
   static NAME* instance()      \
   {                            \
      return &_instance;        \
   }                            \
private:                        \
   NAME();                      \
   NAME(const NAME&)

#endif
