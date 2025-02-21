# Linux Authentication System with Zero-Knowledge Proof

## Description

The current paradigm of linux login procedure is very weak and insecure, a passive key logger could easily record your password.

The idea is that we could utilize an interactive ZKP protocol with an external usb device to authenticate a user. The communication between the usb device and computer will not reveal any information on the password itself. 

A cool feature of this authentication method is that it is compatible with biometric authentication, given that there’s a biometric recognition device that outputs the same value for the same biometric.

The security advantage of this method over the traditional password paradigm is that it prevents man in the middle attack, and this provides some sort of security given that once the physical key is destroyed, no one can recover the password.

## Objectives

We aim at implementing a hardware USB key and the corresponding system programming doing the work described above.

## Planning
