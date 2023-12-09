//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __PROJECT_NODE1_H_
#define __PROJECT_NODE1_H_

#define DATA_TYPE 0
#define ACK_TYPE  1
#define NACK_TYPE 2

#include <omnetpp.h>
#include<iostream>
#include <fstream>
#include <bitset>
#include <vector>
#include <algorithm>
#include "MyMessage_m.h"

using namespace std;
using namespace omnetpp;


/**
 * TODO - Generated class
 */
class Node1 : public cSimpleModule
{
  public:
    vector<string> TypeOfErrorsVector;
    vector<string> DataVector;
    vector<int> Acks;
    int currentSeqNumber=0;
    int expectedSeqNumber=1;
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif
