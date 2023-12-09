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

#include "Node1.h"

Define_Module(Node1);

MyMessage_Base * Create(int currentSeqNumber, vector<string>DataVector)
{
    MyMessage_Base * msg = new MyMessage_Base("");

    string message=DataVector[currentSeqNumber];
    int messageSize=message.size();

    msg->setFrame_Payload(message.c_str());
    msg->setFrame_Type(DATA_TYPE);//data
    msg->setSeq_Num(currentSeqNumber);//awel msg

    //calculate checksum byte
    bitset<8> checkSumByte(0);
    for(int i=0;i<messageSize;i++)
    {
        bitset<8> xbits(message[i]);
        checkSumByte=checkSumByte^xbits;//parity bit
    }
    msg->setChecksum(checkSumByte);

    EV<<"NODE 1 sent message with sequence number ...   ";
    EV << msg->getSeq_Num();
    EV<<"  and payload of ... ";
    EV<< msg->getFrame_Payload();
    EV<<"   and check bits of ...";
    EV<< msg->getChecksum().to_string();
    EV<<endl;
    return msg;

}

void Node1::initialize()
{
    // TODO - Generated method body
    string myText;
    ifstream MyFile("input1.txt");
    if (!MyFile.is_open()) {
        cout << "Error: Unable to open the file 'coordinator.txt'" << endl;
        return;
    }
    while(getline (MyFile, myText))
    {
        string errortype=myText.substr(0,4);
        string data=myText.substr(5,myText.size());
        TypeOfErrorsVector.push_back(errortype);
        DataVector.push_back(data);
    }

    MyFile.close();
}

void Node1::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    if(strcmp(msg->getName(),"start")==0)//msg from coordinator->I am sender
    {
        double delay=double(getParentModule()->par("TD"));
        for(int i=0;i<3;i++)
        {
            MyMessage_Base * msg=Create(currentSeqNumber, DataVector);
            currentSeqNumber++;

            cout<<endl;
            delay+=double(getParentModule()->par("PT"));
            double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));

            scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
            sendDelayed(msg,delay,"out");
        }
    }
    else
    {
        if(msg->isSelfMessage())//timeout
        {
           int seqnumber=atoi(msg->getName());
           MyMessage_Base * msg=Create(seqnumber, DataVector);
           double delay=double(getParentModule()->par("TD"))+double(getParentModule()->par("PT"));
           double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));

           scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
           sendDelayed(msg,delay,"out");
        }
        else
        {

            MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);
            if(mmsg->getFrame_Type()==ACK_TYPE)
            {
               string canceltimer=to_string(mmsg->getSeq_Num()-1);
               cMessage *canceltimercmessage=new cMessage(canceltimer.c_str());
               cancelEvent(canceltimercmessage);
               if(expectedSeqNumber==mmsg->getSeq_Num()) // Ack recieved is equal to ack expected
               {
                   int slidepointer=1; //slide window by one
                   expectedSeqNumber++; // increment expected seq number
                   if(find(Acks.begin(),Acks.end(),expectedSeqNumber)!=Acks.end()) //if the next ack was recieved before
                   {
                      remove(Acks.begin(), Acks.end(), expectedSeqNumber); //remove it from array of acks
                      expectedSeqNumber++;
                      slidepointer++; //slide window again
                      if(find(Acks.begin(),Acks.end(),expectedSeqNumber)!=Acks.end())
                      {
                           remove(Acks.begin(), Acks.end(), expectedSeqNumber);
                           expectedSeqNumber++;
                           slidepointer++;
                      }
                   }
                   double delay=double(getParentModule()->par("TD"));
                   for(int i=0;i<slidepointer;i++) //send new messages after sliding the window
                   {
                      MyMessage_Base * msg=Create(currentSeqNumber, DataVector);
                      currentSeqNumber++;
                      delay+=double(getParentModule()->par("PT"));
                      double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));

                      scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                      sendDelayed(msg,delay,"out");
                   }
               }
               else
               {
                   Acks.push_back(mmsg->getSeq_Num());

               }
            }
            else if(mmsg->getFrame_Type()==NACK_TYPE)
            {
               int seqnumber=mmsg->getSeq_Num();
               MyMessage_Base * msg=Create(seqnumber, DataVector);
               double delay=double(getParentModule()->par("TD"))+double(getParentModule()->par("PT"));
               double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));

               scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
               sendDelayed(msg,delay,"out");

            }
            else
            {
                //Reciever COOOOOOOOOOOOOOOOOOOOOOOOODE
            }
        }
    }
}
