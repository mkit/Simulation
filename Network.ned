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

package backbone;

import ned.DatarateChannel;

network Network
{
    @display("bgb=328,308");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        ICS: PriorityEtherHost {
            parameters:
                @display("p=162,260");
        }
        switch: PriorityEtherSwitch {
            parameters:
                @display("p=162,173");
            gates:
                ethg[4];
        }
        AP1: PriorityEtherSwitch {
            parameters:
                @display("p=63,104");
            gates:
                ethg[4];
        }
        AP2: PriorityEtherSwitch {
            parameters:
                @display("p=162,104");
            gates:
                ethg[4];
        }
        AP3: PriorityEtherSwitch {
            parameters:
                @display("p=261,104");
            gates:
                ethg[4];
        }
        V1: PriorityEtherHost {
            parameters:
                @display("p=63,31");
        }
        V2: PriorityEtherHost {
            parameters:
                @display("p=162,31");
        }
        V3: PriorityEtherHost {
            parameters:
                @display("p=261,31");
        }
    connections allowunconnected:
        switch.ethg[0] <--> C <--> ICS.ethg;
        switch.ethg[1] <--> C <--> AP1.ethg[0];
        switch.ethg[2] <--> C <--> AP2.ethg[0];
        switch.ethg[3] <--> C <--> AP3.ethg[0];
        V1.ethg <--> C <--> AP1.ethg[1];
        V2.ethg <--> C <--> AP2.ethg[1];
        V3.ethg <--> C <--> AP3.ethg[1];
}
