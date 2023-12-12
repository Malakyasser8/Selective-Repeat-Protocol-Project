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

void ByteStuffing(MyMessage_Base * msg,vector<int> &TimeOut)
{
    string newPayloadAfterByteStuffing="#";
    string payload=msg->getFrame_Payload();
    int payloadSize=payload.length();
    for(int i=0;i<payloadSize;i++)
    {
        if(payload[i]=='#' || payload[i]=='/')
            newPayloadAfterByteStuffing+='/';
        newPayloadAfterByteStuffing+=payload[i];
    }
    newPayloadAfterByteStuffing+='#';
    msg->setFrame_Payload(newPayloadAfterByteStuffing.c_str());
    TimeOut.push_back(msg->getSeq_Num());
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
            if(currentSeqNumber>=DataVector.size())
                break;
            MyMessage_Base * msg=Create(currentSeqNumber, DataVector);
            string typeOfError=TypeOfErrorsVector[currentSeqNumber];
            if(typeOfError[1]=='1')//loss -> x1xx
            {
                delay+=double(getParentModule()->par("PT"));
                double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                TimeOut.push_back(msg->getSeq_Num());
                scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
            }
            else if(typeOfError=="0001")//delay
            {
                double tempDelay=delay+double(getParentModule()->par("ED"))+double(getParentModule()->par("PT"));
                double endTimer=tempDelay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                //Byte stuffing
                ByteStuffing(msg,TimeOut);
                sendDelayed(msg,tempDelay,"out");
                delay+=double(getParentModule()->par("PT"));
            }
            else if(typeOfError=="0010")//duplicate
            {
                //original
                delay+=double(getParentModule()->par("PT"));
                double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                //Byte stuffing
                ByteStuffing(msg,TimeOut);
                sendDelayed(msg,delay,"out");
                //dup
                double tempDelay=delay+double(getParentModule()->par("DD"));
                sendDelayed(msg->dup(),tempDelay,"out");
            }
            else if(typeOfError=="1000")//modification
            {
                string msgPayload=msg->getFrame_Payload();
                int errorBit=int(uniform(0,((msgPayload.length())*8)-1));//error in any bit including char count and parity
                int errorChar=errorBit/8;
                errorBit=(errorBit-1)%8;

                bitset<8> erroredCharBits(msgPayload[errorChar]);
                erroredCharBits[errorBit]=!erroredCharBits[errorBit];
                msgPayload[errorChar]=(char)(erroredCharBits).to_ulong() ;
                msg->setFrame_Payload(msgPayload.c_str());

                delay+=double(getParentModule()->par("PT"));
                double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                //Byte stuffing
                ByteStuffing(msg,TimeOut);
                sendDelayed(msg,delay,"out");
            }
            else if(typeOfError=="0000")//no error
            {
                delay+=double(getParentModule()->par("PT"));
                double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                //Byte stuffing
                ByteStuffing(msg,TimeOut);
                //send
                if(i==0)
                    test=msg;
                scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                sendDelayed(msg,delay,"out");
            }
            else if(typeOfError=="0011")//Delay+Duplication
            {
                //delay
                double tempDelay=delay+double(getParentModule()->par("ED"))+double(getParentModule()->par("PT"));
                double endTimer=tempDelay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                //Byte stuffing
                ByteStuffing(msg,TimeOut);
                sendDelayed(msg,tempDelay,"out");
                //dup
                tempDelay+=double(getParentModule()->par("DD"));
                sendDelayed(msg->dup(),tempDelay,"out");
                //lel b3dya
                delay+=double(getParentModule()->par("PT"));
            }
            else if(typeOfError=="1010")//Modification+Duplication
            {
                //modify
                string msgPayload=msg->getFrame_Payload();
                int errorBit=int(uniform(0,((msgPayload.length())*8)-1));//error in any bit including char count and parity
                int errorChar=errorBit/8;
                errorBit=(errorBit-1)%8;

                bitset<8> erroredCharBits(msgPayload[errorChar]);
                erroredCharBits[errorBit]=!erroredCharBits[errorBit];
                msgPayload[errorChar]=(char)(erroredCharBits).to_ulong() ;
                msg->setFrame_Payload(msgPayload.c_str());

                //send first modify
                delay+=double(getParentModule()->par("PT"));
                double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                //Byte stuffing
                ByteStuffing(msg,TimeOut);
                sendDelayed(msg,delay,"out");

                //dup
                double tempDelay=delay+double(getParentModule()->par("DD"));
                sendDelayed(msg->dup(),tempDelay,"out");
            }
            else if(typeOfError=="1011")//Modification+Duplication+Delay
            {
                //modify
                string msgPayload=msg->getFrame_Payload();
                int errorBit=int(uniform(0,((msgPayload.length())*8)-1));//error in any bit including char count and parity
                int errorChar=errorBit/8;
                errorBit=(errorBit-1)%8;

                bitset<8> erroredCharBits(msgPayload[errorChar]);
                erroredCharBits[errorBit]=!erroredCharBits[errorBit];
                msgPayload[errorChar]=(char)(erroredCharBits).to_ulong() ;
                msg->setFrame_Payload(msgPayload.c_str());

                //delay el 1st modified
                double tempDelay=delay+double(getParentModule()->par("ED"))+double(getParentModule()->par("PT"));
                double endTimer=tempDelay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                //Byte stuffing
                ByteStuffing(msg,TimeOut);
                sendDelayed(msg,tempDelay,"out");

                //dup
                tempDelay+=double(getParentModule()->par("DD"));
                sendDelayed(msg->dup(),tempDelay,"out");
                //lel b3dya
                delay+=double(getParentModule()->par("PT"));
            }
            currentSeqNumber++;
        }
    }
    else
    {
        if(msg->isSelfMessage())//timeout
        {
           int seqnumber=atoi(msg->getName());
           if(find(TimeOut.begin(),TimeOut.end(),seqnumber)!=TimeOut.end())
           {
              cout<<"TIMEOUT"<<seqnumber<<endl;
              MyMessage_Base * msg=Create(seqnumber, DataVector);
              double delay=double(getParentModule()->par("TD"))+double(getParentModule()->par("PT"));
              double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
              scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
              TimeOut.erase(remove(TimeOut.begin(), TimeOut.end(),seqnumber));
              ByteStuffing(msg,TimeOut);
              sendDelayed(msg,delay,"out");
           }
        }
        else
        {
            MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);
            if(mmsg->getFrame_Type()==ACK_TYPE)
            {
                EV<<"NODE 1 recieved ack with sequence number ...   ";
                EV << mmsg->getSeq_Num();
                EV<<endl;
                cout<<"TO SIZEE "<<TimeOut.size()<<endl;
                   for(int i=0;i<TimeOut.size();i++)
                  {
                      cout<<"TO BEFORE "<<TimeOut[i]<<endl;
                  }
               if(find(TimeOut.begin(),TimeOut.end(),mmsg->getSeq_Num()-1)!=TimeOut.end())
               {
                   cout<<"REMOVEEE"<<mmsg->getSeq_Num()-1<<endl;
                   TimeOut.erase(remove(TimeOut.begin(), TimeOut.end(), mmsg->getSeq_Num()-1));
                   for(int i=0;i<TimeOut.size();i++)
                   {
                       cout<<"TO AFTER "<<TimeOut[i]<<endl;
                   }
               }
               if(expectedSeqNumber==mmsg->getSeq_Num()) // Ack recieved is equal to ack expected
               {
                   int slidepointer=1; //slide window by one
                   expectedSeqNumber++; // increment expected seq number
                   while(find(Acks.begin(),Acks.end(),expectedSeqNumber)!=Acks.end())//if the next ack was recieved before
                   {
                       Acks.erase(remove(Acks.begin(), Acks.end(), expectedSeqNumber)); //remove it from array of acks
                        expectedSeqNumber++;
                        slidepointer++; //slide window again
                   }
                   double delay=double(getParentModule()->par("TD"));

                   for(int i=0;i<slidepointer;i++) //send new messages after sliding the window
                   {
                      if(currentSeqNumber>=DataVector.size())
                          break;
                      MyMessage_Base * msg=Create(currentSeqNumber, DataVector);
                      string typeOfError=TypeOfErrorsVector[currentSeqNumber];
                      if(typeOfError[1]=='1')//loss -> x1xx
                      {
                          delay+=double(getParentModule()->par("PT"));
                          double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                          scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                      }
                      else if(typeOfError=="0001")//delay
                      {
                          double tempDelay=delay+double(getParentModule()->par("ED"))+double(getParentModule()->par("PT"));
                          double endTimer=tempDelay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                          scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                          //Byte stuffing
                          ByteStuffing(msg,TimeOut);
                          sendDelayed(msg,tempDelay,"out");
                          delay+=double(getParentModule()->par("PT"));
                      }
                      else if(typeOfError=="0010")//duplicate
                      {
                          //original
                          delay+=double(getParentModule()->par("PT"));
                          double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                          scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                          //Byte stuffing
                          ByteStuffing(msg,TimeOut);
                          sendDelayed(msg,delay,"out");
                          //dup
                          double tempDelay=delay+double(getParentModule()->par("DD"));
                          sendDelayed(msg->dup(),tempDelay,"out");
                      }
                      else if(typeOfError=="1000")//modification
                      {
                          string msgPayload=msg->getFrame_Payload();
                          int errorBit=int(uniform(0,((msgPayload.length())*8)-1));//error in any bit including char count and parity
                          int errorChar=errorBit/8;
                          errorBit=(errorBit-1)%8;

                          bitset<8> erroredCharBits(msgPayload[errorChar]);
                          erroredCharBits[errorBit]=!erroredCharBits[errorBit];
                          msgPayload[errorChar]=(char)(erroredCharBits).to_ulong() ;
                          msg->setFrame_Payload(msgPayload.c_str());

                          delay+=double(getParentModule()->par("PT"));
                          double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                          scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                          //Byte stuffing
                          ByteStuffing(msg,TimeOut);
                          sendDelayed(msg,delay,"out");
                      }
                      else if(typeOfError=="0000")//no error
                      {
                          delay+=double(getParentModule()->par("PT"));
                          double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                          //Byte stuffing
                          ByteStuffing(msg,TimeOut);
                          //send
                          scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                          sendDelayed(msg,delay,"out");
                      }
                      else if(typeOfError=="0011")//Delay+Duplication
                      {
                          //delay
                          double tempDelay=delay+double(getParentModule()->par("ED"))+double(getParentModule()->par("PT"));
                          double endTimer=tempDelay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                          scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                          //Byte stuffing
                          ByteStuffing(msg,TimeOut);
                          sendDelayed(msg,tempDelay,"out");
                          //dup
                          tempDelay+=double(getParentModule()->par("DD"));
                          sendDelayed(msg->dup(),tempDelay,"out");
                          //lel b3dya
                          delay+=double(getParentModule()->par("PT"));
                      }
                      else if(typeOfError=="1010")//Modification+Duplication
                      {
                          //modify
                          string msgPayload=msg->getFrame_Payload();
                          int errorBit=int(uniform(0,((msgPayload.length())*8)-1));//error in any bit including char count and parity
                          int errorChar=errorBit/8;
                          errorBit=(errorBit-1)%8;

                          bitset<8> erroredCharBits(msgPayload[errorChar]);
                          erroredCharBits[errorBit]=!erroredCharBits[errorBit];
                          msgPayload[errorChar]=(char)(erroredCharBits).to_ulong() ;
                          msg->setFrame_Payload(msgPayload.c_str());

                          //send first modify
                          delay+=double(getParentModule()->par("PT"));
                          double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                          scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                          //Byte stuffing
                          ByteStuffing(msg,TimeOut);
                          sendDelayed(msg,delay,"out");

                          //dup
                          double tempDelay=delay+double(getParentModule()->par("DD"));
                          sendDelayed(msg->dup(),tempDelay,"out");
                      }
                      else if(typeOfError=="1011")//Modification+Duplication+Delay
                      {
                          //modify
                          string msgPayload=msg->getFrame_Payload();
                          int errorBit=int(uniform(0,((msgPayload.length())*8)-1));//error in any bit including char count and parity
                          int errorChar=errorBit/8;
                          errorBit=(errorBit-1)%8;

                          bitset<8> erroredCharBits(msgPayload[errorChar]);
                          erroredCharBits[errorBit]=!erroredCharBits[errorBit];
                          msgPayload[errorChar]=(char)(erroredCharBits).to_ulong() ;
                          msg->setFrame_Payload(msgPayload.c_str());

                          //delay el 1st modified
                          double tempDelay=delay+double(getParentModule()->par("ED"))+double(getParentModule()->par("PT"));
                          double endTimer=tempDelay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
                          scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
                          //Byte stuffing
                          ByteStuffing(msg,TimeOut);
                          sendDelayed(msg,tempDelay,"out");

                          //dup
                          tempDelay+=double(getParentModule()->par("DD"));
                          sendDelayed(msg->dup(),tempDelay,"out");
                          //lel b3dya
                          delay+=double(getParentModule()->par("PT"));
                      }
                      currentSeqNumber++;
                   }
               }
               else
               {
                   Acks.push_back(mmsg->getSeq_Num());
               }
            }
            else if(mmsg->getFrame_Type()==NACK_TYPE)
            {
                EV<<"NODE 1 recieved NACK with sequence number ...   ";
                EV << mmsg->getSeq_Num();
                EV<<endl;
               int seqnumber=mmsg->getSeq_Num();
               MyMessage_Base * msg=Create(seqnumber, DataVector);
               double delay=double(getParentModule()->par("TD"))+double(getParentModule()->par("PT"));
               double endTimer=delay - double(getParentModule()->par("TD")) + double(getParentModule()->par("TO"));
               scheduleAt(simTime()+endTimer,new cMessage((to_string(currentSeqNumber)).c_str()));
               ByteStuffing(msg,TimeOut);
               sendDelayed(msg,delay,"out");
            }
            else
            {
                //Reciever COOOOOOOOOOOOOOOOOOOOOOOOODE
            }
        }
    }
}
