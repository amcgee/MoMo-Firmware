//i2c_defines.h
/*
 * All i2c constant values for sharing between c and asm code.
 */

#ifndef __i2c_defines_h__
#define __i2c_defines_h__


#define kI2CIdleState 				0
#define kI2CSendAddressState 		1
#define kI2CSendDataState 			2
#define kI2CSendChecksumState 		3
#define kI2CReceiveDataState 		4
#define kI2CReceiveChecksumState 	5
#define kI2CUserCallbackState 		6
#define kI2CReceivedChecksumState 	7

#define kI2CNoError 				0
#define kI2CInvalidChecksum 		1
#define kI2CNackReceived 			2
#define kI2CCollision 				3

#define kDisableI2CSlewControlFlag 	(1 << 7)
#define kSMBusLevelCompliantFlag 	(1 << 6)

#define kI2CFlagMask (kDisableI2CSlewControlFlag | kSMBusLevelCompliantFlag)

#define pack_i2c_states(master, send, next, done) (((master&1)<<7) | ((send&1)<<3) | (next&0b111) | ((done&0b111)<<4))

//both set CKP and SSPEN
#define kI2CMasterMode 				0b00111000            //set to master mode (0b1000)
#define kI2CSlaveMode 		        0b00110110            //set to slave mode (0b0110)


#endif