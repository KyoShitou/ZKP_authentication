# Linux Authentication System with Zero-Knowledge Proof

## Description

The current paradigm of linux login procedure is very weak and insecure, a passive key logger could easily record your password.

The idea is that we could utilize an interactive ZKP protocol with an external usb device to authenticate a user. The communication between the usb device and computer will not reveal any information on the password itself.

A cool feature of this authentication method is that it is compatible with biometric authentication, given that there’s a biometric recognition device that outputs the same value for the same biometric.

The security advantage of this method over the traditional password paradigm is that it prevents man in the middle attack, and this provides some sort of security given that once the physical key is destroyed, no one can recover the password.

### Advantage

In addition to the current paradigm of RSA authentication, ZKP allows the key to proof more information on the user's identity,
for example, a key specifying the priviledge of running certain tasks or utilizing certain resources ("The bearer of this key is permitted to use 4 GPUs installed on this system")

### Method

In this project, we plan to use the following protocol proposed by Chaum & Evertse.
The key $x$ will hold the discrete logarithm value stored in the host machine, more
formally, host machine will store $y$, and the key will need to proof that they possess
$x$ such that $g^x \equiv y \mod p$, where $g, p$ is pre-agreed in the protocol.

The proof will be in multiple rounds, in each round:

The key selects a random number $r$, computes $C = g^r \mod p$, send $C$ to the host system.

The host machine randomly request that the key either disclose $r$, or disclose the value
$(x + r) \mod (p - 1)$, the key will act accordingly.

The host system then could verify the value key returned, if host system requested $r$, trivially
test $C \overset{?}{=} g^r \mod p$, else if the host system requested $(x + r) \mod (p - 1)$, they could verify by $(C \cdot y) \mod p \overset{?}{=} g^{(x + r) \mod (p - 1)}\mod p$

## Objectives

We aim at implementing a hardware USB key and the corresponding system programming doing the work described above.

## Planning

In case we will not be able to finish everything in time, we prioritize having functioning outcome as soon as possbile. Therefore, we would focus on making the software part (namely the system programming part).

**1st week - Logistics:** Get a Raspberry Pi, building up the development environment (namely docker)

**2nd week to 5th week - Software:** Implement the software part, inlcuding ZKP protocol and integrated with Linux system

**6th week to 9th week - Hardware:** Implement the firmware of the USB key

## How to compile

Run the following command in `Software/src/` directory.


```
gcc -fPIC -c PAM_Module.c -o PAM_Module.o
gcc -shared -o pam_zkp.so PAM_Module.o -lpam

```
