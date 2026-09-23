/**
 * @file mikai.h
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-23
 */
#ifndef MIKAI_H
#define MIKAI_H

class Mikai {
public:
    /////////////////////////////////////////////////////////////////////////////////////
    // Constructor
    /////////////////////////////////////////////////////////////////////////////////////
    Mikai();

private:
    void readTag();
    void writeModified();
    void showInfo();
    void addCredit();
    void setCredit();
};

#endif // MIKAI_H
