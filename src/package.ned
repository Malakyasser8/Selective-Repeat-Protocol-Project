package project;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    parameters:
        int WS;
        int WR;
        double TO;
        double PT;
        double TD;
        double ED;
        double DD;
    @display("bgb=801,247");
    submodules:
        coordinator: Coordinator {
            @display("p=367,46");
        }
        node0: Node0 {
            @display("p=216,140");
        }
        node1: Node1 {
            @display("p=521,139");
        }
    connections:
        coordinator.outs++ --> node0.ins++;
        coordinator.outs++ --> node1.ins++;
        node0.out --> node1.ins++;
        node1.out --> node0.ins++;
}

