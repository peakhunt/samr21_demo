#ifndef __802_15_4_MAC_DEF_H__
#define __802_15_4_MAC_DEF_H__

#include <stdint.h>

#define _802_15_4_MAX_MSDU_BUF_SIZE           127

typedef uint32_t _802_15_4_timestamp_t;

typedef enum
{
  _802_15_4_Address_Mode_Reserved = 0x01,
  _802_15_4_Address_Mode_16_Bit   = 0x02,
  _802_15_4_Address_Mode_64_Bit   = 0x03,
} _802_15_4_Address_Mode_t;

typedef enum
{
  _802_15_4_Coord_Address_Mode_Short  = 0x02,
  _802_15_4_Coord_Address_Mode_Long   = 0x03,
} _802_15_4_Coord_Address_Mode_t;

typedef enum
{
  _802_15_4_Status_SUCCESS        = 0,
  _802_15_4_Status_TRANSACTION_OVERFLOW,
  _802_15_4_Status_TRANSACTION_EXPIRED,
  _802_15_4_Status_CHANNEL_ACCESS_FAILURE,
  _802_15_4_Status_INVALID_ADDRESS,
  _802_15_4_Status_INVALID_GTS,
  _802_15_4_Status_NO_ACK,
  _802_15_4_Status_COUNTER_ERROR,
  _802_15_4_Status_FRAME_TOO_LONG,
  _802_15_4_Status_UNAVAILABLE_KEY,
  _802_15_4_Status_UNSUPPORTED_SECURITY,
  _802_15_4_Status_INVALID_PARAMETER,
  _802_15_4_Status_INVALID_HANDLE,
} _802_15_4_Status_t;

#define _802_15_4_Tx_Options_UnAck    0x00
#define _802_15_4_Tx_Options_Ack      0x04
#define _802_15_4_Tx_Options_GTS      0x02
#define _802_15_4_Tx_Options_CAP      0x00
#define _802_15_4_Tx_Options_Indirect 0x01
#define _802_15_4_Tx_Options_Direct   0x00

struct ____802_15_4_mac;
typedef struct ____802_15_4_mac __802_15_4_mac_t;

typedef struct
{
  _802_15_4_Address_Mode_t    srcAddrMode;
  uint16_t                    srcPanID;
  uint8_t                     srcAddr[8];

  _802_15_4_Address_Mode_t    dstAddrMode;
  uint16_t                    dstPanID;
  uint8_t                     dstAddr[8];

  uint8_t                     msduLen;
  uint8_t                     msdu[_802_15_4_MAX_MSDU_BUF_SIZE];
  uint8_t                     mpduLinkQuality;

  uint8_t                     dsn;
  _802_15_4_timestamp_t       timestamp;

  uint8_t                     secLevel;
  uint8_t                     keyIdMode;
  uint8_t                     keySource[8];
  uint8_t                     keyIndex;
} __802_15_4_mac_data_indication_t;

typedef struct
{
  _802_15_4_Address_Mode_t    dstAddrMode;
  uint16_t                    dstPanID;
  uint8_t                     dstAddr[8];

  _802_15_4_Address_Mode_t    srcAddrMode;

  uint8_t                     msduLen;
  uint8_t                     msdu[_802_15_4_MAX_MSDU_BUF_SIZE];
  uint8_t                     msduHandle;

  uint8_t                     txOptions;
  uint8_t                     secLevel;
  uint8_t                     keyIdMode;
  uint8_t                     keySource[8];
  uint8_t                     keyIndex;
} __802_15_4_mac_data_request_t;

typedef struct
{
  uint8_t                           logicalChannel;
  uint8_t                           channelPage;
  _802_15_4_Coord_Address_Mode_t    coordAddrMode;
  uint16_t                          coordPanId;
  uint8_t                           coordAddress[8];
  uint8_t                           capInfo;

  uint8_t                           secLevel;
  uint8_t                           keyIdMode;
  uint8_t                           keySource[8];
  uint8_t                           keyIndex;
} __802_15_4_mac_mlme_assoc_request_t;

typedef struct
{
  uint8_t                           deviceAddr[8];
  uint8_t                           capInfo;
  uint8_t                           secLevel;
  uint8_t                           keyIdMode;
  uint8_t                           keySource[8];
  uint8_t                           keyIndex;
} __802_15_4_mac_mlme_assoc_indication_t;

typedef struct
{
  uint8_t                           deviceAddr[8];
  uint8_t                           assocShortAddress[2];
  _802_15_4_Status_t                status;
  uint8_t                           secLevel;
  uint8_t                           keyIdMode;
  uint8_t                           keySource[8];
  uint8_t                           keyIndex;
} __802_15_4_mac_mlme_assoc_response_t;

typedef struct
{
  uint8_t                           assocShortAddress[2];
  _802_15_4_Status_t                status;
  uint8_t                           secLevel;
  uint8_t                           keyIdMode;
  uint8_t                           keySource[8];
  uint8_t                           keyIndex;
} __802_15_4_mac_mlme_assoc_confirm_t;

//
// Data Service Callbacks
//
typedef void (*__802_15_4_mac_data_indication)(__802_15_4_mac_t* mac,
    __802_15_4_mac_data_indication_t* ind);
typedef void (*__802_15_4_mac_data_confirm)(__802_15_4_mac_t* mac,
    uint8_t msuHandle,
    _802_15_4_Status_t status,
    _802_15_4_timestamp_t timestamp);
typedef void (*__802_15_4_mac_purge_confirm)(__802_15_4_mac_t* mac, uint8_t msduHandle, _802_15_4_Status_t status);

//
// MLME Callbacks
//
typedef void (*__802_15_4_mac_mlme_assoc_indication)(__802_15_4_mac_t* mac,
    __802_15_4_mac_mlme_assoc_indication_t* ind);
typedef void (*__802_15_4_mac_mlme_assoc_confirm)(__802_15_4_mac_t* mac,
    __802_15_4_mac_mlme_assoc_confirm_t* cfm);

struct ____802_15_4_mac
{
  __802_15_4_mac_data_indication        data_indication;
  __802_15_4_mac_data_confirm           data_confirm;
  __802_15_4_mac_purge_confirm          purge_confirm;

  __802_15_4_mac_mlme_assoc_indication  assoc_indication;
  __802_15_4_mac_mlme_assoc_confirm     assoc_confirm;

  uint8_t                         srcAddrShort[2];
  uint8_t                         srcAddrLong[8];
};

extern void __802_15_4_mac_data_request(__802_15_4_mac_t* mac, __802_15_4_mac_data_request_t* req);
extern void __802_15_4_mac_purge_request(__802_15_4_mac_t* mac, uint8_t msduHandle);

extern void __802_15_4_mac_assoc_request(__802_15_4_mac_t* mac, __802_15_4_mac_mlme_assoc_request_t* req);
extern void __802_15_4_mac_assoc_response(__802_15_4_mac_t* mac, __802_15_4_mac_mlme_assoc_response_t* rsp);


#endif /* !__802_15_4_MAC_DEF_H__ */
