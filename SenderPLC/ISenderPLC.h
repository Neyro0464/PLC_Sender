#ifndef ISENDERPLC_H
#define ISENDERPLC_H

class ISenderPLC {
public:
    virtual ~ISenderPLC() = default;
    virtual void send() = 0;
    virtual void setDestination() = 0;


};

#endif // ISENDERPLC_H
