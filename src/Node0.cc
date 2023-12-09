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

#include "Node0.h"

Define_Module(Node0);

void Node0::initialize()
{
    // TODO - Generated method body
    cout<<"HI0"<<endl;
}

void Node0::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);
    mmsg->setFrame_Payload("");
    mmsg->setFrame_Type(2);//data
    mmsg->setSeq_Num(mmsg->getSeq_Num());
    send(mmsg,"out");
}
