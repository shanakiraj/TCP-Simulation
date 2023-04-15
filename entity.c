/******************************************************************************/
/*                                                                            */
/* ENTITY IMPLEMENTATIONS                                                     */
/*                                                                            */
/******************************************************************************/

// Student names: Shan Akiraj
// Student computing IDs: sra9qsw
//
//
// This file contains the actual code for the functions that will implement the
// reliable transport protocols enabling entity "A" to reliably send information
// to entity "B".
//
// This is where you should write your code, and you should submit a modified
// version of this file.
//
// Notes:
// - One way network delay averages five time units (longer if there are other
//   messages in the channel for GBN), but can be larger.
// - Packets can be corrupted (either the header or the data portion) or lost,
//   according to user-defined probabilities entered as command line arguments.
// - Packets will be delivered in the order in which they were sent (although
//   some can be lost).
// - You may have global state in this file, BUT THAT GLOBAL STATE MUST NOT BE
//   SHARED BETWEEN THE TWO ENTITIES' FUNCTIONS. "A" and "B" are simulating two
//   entities connected by a network, and as such they cannot access each
//   other's variables and global state. Entity "A" can access its own state,
//   and entity "B" can access its own state, but anything shared between the
//   two must be passed in a `pkt` across the simulated network. Violating this
//   requirement will result in a very low score for this project (or a 0).
//
// To run this project you should be able to compile it with something like:
//
//     $ gcc entity.c simulator.c -o myproject
//
// and then run it like:
//
//     $ ./myproject 0.0 0.0 10 500 3 test1.txt
//
// Of course, that will cause the channel to be perfect, so you should test
// with a less ideal channel, and you should vary the random seed. However, for
// testing it can be helpful to keep the seed constant.
//
// The simulator will write the received data on entity "B" to a file called
// `output.dat`.

#include <stdio.h>
#include "simulator.h"

//Global
struct pkt packet;
int count;

//keep track of window
int send_base;

//create the buffer
struct pkt buffer[10000];

//keep track of nextseqnum
int nextseqnum;

/**** A ENTITY ****/
void A_init() {
    nextseqnum = 0;
    send_base = 0;
    count = 0;
}

void A_output(struct msg message) {

    //create packet
    packet.seqnum = nextseqnum;
    packet.acknum = nextseqnum;
    packet.length = message.length;
    packet.checksum = packet.seqnum + packet.acknum + packet.length;

    for (int i = 0; i < 20; i++) {
        packet.checksum += message.data[i] - '0';
        packet.payload[i] = message.data[i];
    }
    //add the packet to the buffer
    buffer[count] = packet;
    
    //send the packet to B if in window
    if(nextseqnum < send_base + 8){
        //send the packet
        tolayer3_A(packet);

        //only start timer for first packet in window
        if(send_base == nextseqnum){
            starttimer_A(200.0);
        }
        //increase nextseqnum
        nextseqnum++;
    }
    count++;
    
}

void A_input(struct pkt packet) {

    //check for corruption
    int sum = packet.acknum + packet.length + packet.seqnum;
    for (int i = 0; i < 20; i++) {
            sum += packet.payload[i] - '0';
        }

    if(packet.acknum >= send_base && packet.checksum == sum){ 
        
        //update the sendbase
        send_base = packet.acknum + 1;

        //check if last packet has been recieved in window
        if(send_base == nextseqnum){
            stoptimer_A();
        }else{ //check if everything in window has been sent

            starttimer_A(200.0);
            // A_output(messages[nextseqnum]);
            
        }
    
    }
}

void A_timerinterrupt() {
    
    starttimer_A(200.0);

    int temp = nextseqnum;

    for (int i = send_base; i < temp; i++) {
        //send the packet
        tolayer3_A(buffer[i]);
    }
    
}


/**** B ENTITY ****/

int recv_base;

void B_init() {
    recv_base = 0;
}

void B_input(struct pkt packet) {

    //check if corrupt
    int sum = packet.acknum + packet.length + packet.seqnum;
    
    for (int i = 0; i < 20; i++) {
            sum += packet.payload[i] - '0';
        }

    if(packet.checksum == sum){
        //the packet is not corrupt

        if(packet.acknum == recv_base){
            //packet was in order as well

            tolayer3_B(packet);
            recv_base++;

            struct msg message;
            message.length = packet.length;
            for (int i=0; i<20; i++) {
                message.data[i] = packet.payload[i];
            }
            //sends it to layer 5
            tolayer5_B(message);
        }else{//a packet was not recieved
            //send the duplicate ack number
            packet.acknum = recv_base-1;
            tolayer3_B(packet);
        }
        
    }
            
    
}

void B_timerinterrupt() {

}
