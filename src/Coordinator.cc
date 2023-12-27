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

#include "Coordinator.h"

Define_Module(Coordinator);

void Coordinator::initialize()
{
    // TODO - Generated method body
    string myText;
    ifstream MyFile("coordinator.txt");
    if (!MyFile.is_open()) {
        cout << "Error: Unable to open the file 'coordinator.txt'" << endl;
        return;
    }
    getline (MyFile, myText);
    int nodeNumber=int(myText[0]-'0');
    int startTime=int(myText[2]-'0');
    cout<<"String"<<myText<<endl;
    cout<<"node number  "<<nodeNumber<<"  start time  "<<startTime<<endl;
    MyFile.close();
    scheduleAt(simTime()+startTime,new cMessage((to_string(nodeNumber)).c_str()));
    outputFile.open("output.txt");
    outputFile.close();

}

void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    if(msg->isSelfMessage())
    {
        if(strcmp(msg->getName(),"0")==0)
        {
            cout<<"coordinator sending msg to node 0"<<endl;
            msg->setName("start");
            send(msg,"outs",0);
        }
        else if(strcmp(msg->getName(),"1")==0)
        {
            cout<<"coordinator sending msg to node 1"<<endl;
            msg->setName("start");
            send(msg,"outs",1);
        }
    }
}
