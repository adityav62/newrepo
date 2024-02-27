
#include <iostream>
#include <systemc.h>
#include <queue>

using namespace std;

//Interface definition - derived from sc_interface
//without this, SystemC won't recognise our interface properly
template <class T>
class SImpleFIFOInterface: public sc_interface
{
    public: //methods defined in Interface must be pure virtual
    virtual T read() = 0;
    virtual void write(T) = 0;
};

//since we derive FIFO from Interface, the FIFO class 
//itself - the channel in this case, has to be templated
template<class T>
class SimpleFIFO: public SImpleFIFOInterface<T>
{   
    //only our class itself is allowed to access 
    //actual FIFO, which holds actual data
    private: 
    queue<T> fifo;
    sc_event writtenEvent; //will be notified when sth is written
    sc_event readEvent; //will be notified when sth is read
    //events will be used later on to unlock the Producer and Consumer
    uint64_t maxSize; //size of the queue

    public:
    SimpleFIFO(uint64_t size = 16): maxSize(size){} //constructor definition, which gets the size as an argument and stores it in maxSize
    
    //we need to implement the pure virtual functions defined in interface
    //In read, wait until there is sth in queue - if queue empty, wait, else read from it
    T read() 
    {
        if(fifo.empty() == true )
        {
            wait(writtenEvent); //wait (blocked) until something is written - when something is written, code will continue
        }
        T val = fifo.front(); //store the first element of FIFO in val and pop it
        fifo.pop();
        readEvent.notify(SC_ZERO_TIME); //when queue is full and someone reads from it, there is new space to whcih writing can be done
        return val;
    }

    void write(T val)
    {   //there is no fifo.full()
        if(fifo.size() == maxSize)
        {
            wait(readEvent); //wait until another consumer or process has read from the FIFO
        }
        fifo.push(val);
        writtenEvent.notify(SC_ZERO_TIME);
    }
};

//After creating Interfaces and Channels, we need some Producers and Consumers
//producer will have a port that is defined by the interface
SC_MODULE(PRODUCER)
{
    sc_port<SImpleFIFOInterface<int> > master;

    SC_CTOR(PRODUCER)
    {
        SC_THREAD(process);
    }

    void process()
    {
        int counter = 0;
        while(true)
        {
            wait(1,SC_NS);
            cout<< "@" << sc_time_stamp() << " P: " << counter <<endl;
            master->write(counter++); //we need to use this for calling interface methods with standard ports
        }
    }
};

//Consumer similar to Producer
SC_MODULE(CONSUMER)
{
sc_port<SImpleFIFOInterface<int> > slave;

    SC_CTOR(CONSUMER)
    {
        SC_THREAD(process);
    }

    void process()
    {
        while(true)
        {
            wait(4,SC_NS); //to see if the queing is happening, we keep the rate different
            cout<< "@" << sc_time_stamp() << " C: " <<slave->read() << endl;
        }
    }
};

int sc_main(int __attribute__((unused)) argc, char __attribute__((unused)) *argv[])
{
    PRODUCER p1("Producer1");
    CONSUMER c1("Consumer1");

    SimpleFIFO<int> channel(4); // FIFO is our channel in which we want to have 4 elements
    
    p1.master.bind(channel);
    c1.slave.bind(channel);

    sc_start(100, SC_NS);

    return 0;
}

