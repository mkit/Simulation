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

#include "PriorityFrameEncap.h"
#include "PriorityEtherFrame_m.h"

Define_Module(PriorityFrameEncap);

simsignal_t PriorityFrameEncap::encapPkSignal = SIMSIGNAL_NULL;
simsignal_t PriorityFrameEncap::decapPkSignal = SIMSIGNAL_NULL;

void PriorityFrameEncap::initialize()
{
    seqNum = 0;
    WATCH(seqNum);

    totalFromHigherLayer = totalFromMAC = 0;

    encapPkSignal = registerSignal("encapPk");
    decapPkSignal = registerSignal("decapPk");

    maxPriority = par("maxPriority");

    WATCH(totalFromHigherLayer);
    WATCH(totalFromMAC);
}

void PriorityFrameEncap::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("lowerLayerIn"))
    {
        processFrameFromMAC(check_and_cast<EtherFrame *>(msg));
    }
    else
    {
        // from higher layer
        switch (msg->getKind())
        {
            case IEEE802CTRL_DATA:
            case 0: // default message kind (0) is also accepted
              processPacketFromHigherLayer(PK(msg));
              break;
            default:
              throw cRuntimeError("Received message `%s' with unknown message kind %d", msg->getName(), msg->getKind());
        }
    }

    if (ev.isGUI())
        updateDisplayString();
}

void PriorityFrameEncap::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "passed up: %ld\nsent: %ld", totalFromMAC, totalFromHigherLayer);
    getDisplayString().setTagArg("t", 0, buf);
}

void PriorityFrameEncap::processPacketFromHigherLayer(cPacket *msg)
{
    if (msg->getByteLength() > MAX_ETHERNET_DATA_BYTES)
        error("packet from higher layer (%d bytes) exceeds maximum Ethernet payload length (%d)", (int)msg->getByteLength(), MAX_ETHERNET_DATA_BYTES);

    totalFromHigherLayer++;
    emit(encapPkSignal, msg);

    // Creates MAC header information and encapsulates received higher layer data
    // with this information and transmits resultant frame to lower layer

    // create Ethernet frame, fill it in from Ieee802Ctrl and encapsulate msg in it
    EV << "Encapsulating higher layer packet `" << msg->getName() <<"' for MAC\n";

    Ieee802Ctrl *etherctrl = check_and_cast<Ieee802Ctrl*>(msg->removeControlInfo());

    PriorityEtherFrame *frame = new PriorityEtherFrame(msg->getName());

    frame->setSrc(etherctrl->getSrc());  // if blank, will be filled in by MAC
    frame->setDest(etherctrl->getDest());

    //Priority setting
    frame->setPriority(intrand(maxPriority));


    frame->setByteLength(ETHER_MAC_FRAME_BYTES + PRIORITY_ETHER_MAC_FRAME_BYTES);

    delete etherctrl;

    EV << "Encapsulating higher layer packet `" << msg->getName() <<"' for MAC\n";
    EV << "Frame priority: " << frame->getPriority() << endl;

    frame->encapsulate(msg);
    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);  // "padding"

    send(frame, "lowerLayerOut");
}

void PriorityFrameEncap::processFrameFromMAC(EtherFrame *frame)
{
    // decapsulate
    cPacket *higherlayermsg = frame->decapsulate();

    // add Ieee802Ctrl to packet
    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setSrc(frame->getSrc());
    etherctrl->setDest(frame->getDest());
    if (dynamic_cast<PriorityEtherFrame *>(frame) != NULL)
        etherctrl->setEtherType(((EthernetIIFrame *)frame)->getEtherType());
    higherlayermsg->setControlInfo(etherctrl);

    EV << "Decapsulating frame `" << frame->getName() <<"', passing up contained "
          "packet `" << higherlayermsg->getName() << "' to higher layer\n";

    totalFromMAC++;
    emit(decapPkSignal, higherlayermsg);

    // pass up to higher layers.
    send(higherlayermsg, "upperLayerOut");
    delete frame;
}
