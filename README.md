<!--
Copyright (c) 2023-2028, Hossein Moein
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of Hossein Moein and/or the ThreadPool nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Hossein Moein BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-->
<img src="docs/Leopard.jpg" alt="ThreadPool Leopard" width="400" longdesc="https://htmlpreview.github.io/?https://github.com/hosseinmoein/ThreadPool/blob/master/README.md"/>

This is a light-weight C++ Thread Pool that allows object member functions to run on threads.<BR>
It has the following features:<BR>
1. It is templated with a class type. You can only run that class' member functions on the thread pool.
2. The thread pool is constructed with a set number of threads that will be in waiting state.
3. You `dispatch` a routine -- member function.
   1. If there is a thread available, your routine runs on the available thread.
   2. If there is no thread available, your routine will be added to a queue until a thread becomes available.
   3. `dispatch` has an optional Boolean argument named `immediately`. If it is true and no thread is available, a thread will be added to the pool and your routine will execute immediately.
4. At any point you can add/subtract threads to/from the pool.
5. You can set a timeout. So if any thread does nothing but waiting for the timeout period, it will terminate. 
6. The destructor of the thread pool waits until all threads are done running routines.

```cpp
struct   MyClass  {
    bool routine ()  {
    
        std::cout << "From routine()\n";
        ::sleep(2);
        return (true);
    }
private:
    int data { }
};

// -------------------------------------------------------------

using ThreadPoolType = ThreadPool<MyClass>;
MyClass my_obj;

int main (int, char *[])  {

    // Start off with 5 threads in reserve in the pool
    // "timeout_flag" is true by default
    // "timeout_time" is set to half an hour for idle treads by default
    //
    ThreadPoolType  thr_pool (5);

    thr_pool.add_thread (2);  // Add 2 threads to the pool
    thr_pool.add_thread (-3); // Terminate 3 threads in the pool

    // Dispatch routine from the my_obj instance
    // "immediately" flag is set to false by default
    //
    thr_pool.dispatch (&my_obj, &MyClass::routine);

    std::cout << "Available threads: " << thr_pool.available_threads() << std::endl;
    return (EXIT_SUCCESS);
}
```
