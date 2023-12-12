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
    recievedFrames.push_back("");
    recievedFrames.push_back("");
    recievedFrames.push_back("");
}

void Node0::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);
    if(mmsg->getFrame_Type()==DATA_TYPE)
    {
        //recived galy msg ha check errord b checksum(b3d ma asheel el control bytes) law errored->nack law la ack
        //law ack: law heya el expected -> shift window cout el amltelha shift
        //w b3den abos al vector law el b3dya mawgoda ha cout w a set el mkann el fl vector b empty string aw ayan kan
        //w abos al b3dha ->>> while loop
        // # h//icdvfivfh //////# #
        string payload=mmsg->getFrame_Payload();
        string newPayload="";
        int payloadSize=payload.length();
        for(int i=1;i<payloadSize-1;i++)
        {
            if((payload[i]=='/' ))
            {
                newPayload+=payload[i+1];
                i++;
            }
            else
                newPayload+=payload[i];
        }
        EV<<"NODE 0 recieved message with sequence number ...   ";
        EV << mmsg->getSeq_Num();
        EV<<"  and payload of ... ";
        EV<< mmsg->getFrame_Payload();
        EV<<"   and check bits of ...";
        EV<< mmsg->getChecksum().to_string();
        EV<<"   and at time ...";
        EV<< simTime();
        EV<<endl;
        payloadSize=newPayload.length();
        bitset<8> checkSumByte(0);
        for(int i=0;i<payloadSize;i++)
        {
            bitset<8> xbits(newPayload[i]);
            checkSumByte=checkSumByte^xbits;//parity bit
        }
        if(checkSumByte == mmsg->getChecksum())
        {
            cout<<"CHECKSUM SAHHH"<<endl;
            int recievedSeqNum=mmsg->getSeq_Num();
            cout<<"RECIEVED SEQ"<<recievedSeqNum<<"EXPECTED SEQ"<<expectedSeqNumber<<endl;
            if(recievedSeqNum<expectedSeqNumber)
                return;
            else if(recievedSeqNum==expectedSeqNumber)
            {
                int slidepointer=1; //slide window by one
                expectedSeqNumber++;
                cout<<"RECIEVED PAYLOAAD: "<<newPayload<<endl;
                while(recievedFrames[expectedSeqNumber%3]!="")
               {
                    cout<<"RECIEVED PAYLOAAD: "<<recievedFrames[expectedSeqNumber%3]<<endl;
                    recievedFrames[expectedSeqNumber%3]="";
                    expectedSeqNumber++;
                    slidepointer++; //slide window again
               }
            }
            else
            {
                if(recievedFrames[recievedSeqNum%3]!="")
                    return;
                recievedFrames[recievedSeqNum%3]=newPayload;
            }
            mmsg->setFrame_Type(ACK_TYPE);//ACK
            mmsg->setSeq_Num(mmsg->getSeq_Num()+1);
        }
        else
        {
            mmsg->setFrame_Type(NACK_TYPE);//NACK
            mmsg->setSeq_Num(mmsg->getSeq_Num());
        }
        mmsg->setFrame_Payload("");
        sendDelayed(mmsg,double(getParentModule()->par("TD")),"out");
    }

}
