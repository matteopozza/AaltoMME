/* AaltoMME - Mobility Management Entity for LTE networks
 * Copyright (C) 2013 Vicent Ferrer Guash & Jesus Llorente Santos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   EPS_Session.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  NAS ESM implementation
 * @brief  EPS_Session Information
 *
 * This module implements the EPS_Session Information retrived from the HSS
 */

#include <string.h>
#include "EPS_Session_priv.h"
#include "NAS_ESM_priv.h"
#include "NAS_EMM_priv.h"
#include "logmgr.h"
#include "MME_S11.h"
#include "S1AP.h"

EPS_Session ePSsession_init(ESM esm, Subscription _subs, ESM_BearerContext b){
    EPS_Session_t *self = g_new0(EPS_Session_t, 1);
    self->esm = esm;
    self->subs = _subs;
    self->defaultBearer = b;
    self->bearers = g_hash_table_new_full (g_int_hash,
                                           g_int_equal,
                                           NULL,
                                           esm_bc_free);
    self->current_pti = 0;
    return self;
}

void ePSsession_free(EPS_Session s){
    EPS_Session_t *self = (EPS_Session_t*)s;
    if(self->APN){
        g_string_free(self->apn_io_replacement, TRUE);
    }
    if(self->subscribedAPN){
        g_string_free(self->apn_io_replacement, TRUE);
    }
    if(self->apn_io_replacement){
        g_string_free(self->apn_io_replacement, TRUE);
    }
    g_hash_table_destroy(self->bearers);
    g_free(self);
}

void ePSsession_parsePDNConnectivityRequest(EPS_Session s, ESM_Message_t *msg,
                                            gboolean *infoTxRequired){
    uint8_t  *pointer, numOp=0;
    ie_tlv_t4_t *temp;
    EPS_Session_t *self = (EPS_Session_t*)s;
    PDNConnectivityRequest_t *pdnReq = (PDNConnectivityRequest_t*)msg;
    union nAS_ie_member const *optIE=NULL;

    *infoTxRequired = FALSE;
    self->current_pti = msg->procedureTransactionIdentity;
    /*Optionals*/
    /* ESM information transfer flag: 0xD*/
    nas_NASOpt_lookup(pdnReq->optionals, 4, 0xD, &optIE);
    if(optIE){
        *infoTxRequired = (gboolean)optIE->v_t1_l.v;
    }
    /*Access point name: 0x28*/
    /*Protocol configuration options: 0x27*/
    nas_NASOpt_lookup(pdnReq->optionals, 4, 0x27, &optIE);
    if(optIE){
        ePSsession_setPCO(self, &(optIE->tlv_t4.v), optIE->tlv_t4.l);
    }
    /*Device properties: 0xC*/
}

void ePSsession_sendActivateDefaultEPSBearerCtxtReq(EPS_Session s){
    EPS_Session_t *self = (EPS_Session_t*)s;

    /* TODO: verify APN max and PCO length (1 byte) conflict */
    guint8 apn[256]; /* MAX DNS name length */
    guint8 buffer[356], *pointer, pco[0xff+2];
    gsize len;
    gboolean pco_exists = FALSE;

    struct qos_t qos;
    struct PAA_t paa;

    memset(buffer, 0, 150);
    pointer = buffer;

    /* Activate default */
    newNASMsg_ESM(&pointer,
                  EPSSessionManagementMessages,
                  esm_bc_getEBI(self->defaultBearer));
    encaps_ESM(&pointer, self->current_pti,
               ActivateDefaultEPSBearerContextRequest);

    /* EPS QoS */
    subs_cpyQoS_GTP(self->subs, &qos);
    nasIe_lv_t4(&pointer, &(qos.qci), 1);

    /* Access point name*/
    len=0;
    subs_getEncodedAPN(self->subs, apn, 150, &len);
    nasIe_lv_t4(&pointer,
                apn,
                len);

    /* PDN address */
    memset(&paa, 0, sizeof(struct PAA_t));
    paa.type = self->pdn_addr_type;
    switch(self->pdn_addr_type) {
    case 1: /*IPv4*/
        memcpy(&(paa.addr.ipv4), self->pdn_addr, 4);
        len=5;
        break;
    case 2: /*IPv6*/
        memcpy(&(paa.addr.ipv6), self->pdn_addr, 16);
        len=17;
        break;
    case 3: /* IPv4v6*/
        memcpy(&(paa.addr.both.ipv4), self->pdn_addr, 4);
        memcpy(&(paa.addr.both.ipv6), self->pdn_addr+4, 16);
        len=21;
        break;
    default:
        g_error("Wrong PDN address type %u", self->pdn_addr_type);
    }
    nasIe_lv_t4(&pointer, (uint8_t*)&paa, len);

    /*Optionals */

    /* Protocol Configuration Options*/
    pco_exists = ePSsession_getPCO(self, pco, &len);
    if(pco_exists){
        nasIe_tlv_t4(&pointer, 0x27, pco, len);
    }

    emm_attachAccept(self->esm->emm, buffer, pointer-buffer,
                     g_list_append(NULL, self));
}

void ePSsession_activateDefault(EPS_Session s, gboolean infoTxRequired){
    EPS_Session_t *self = (EPS_Session_t*)s;
    guint8 *pointer, buf[100];
    if(!infoTxRequired){
        self->s11 = S11_newUserAttach(esm_getS11iface(self->esm),
                                      self->esm->emm, self,
                                      ePSsession_sendActivateDefaultEPSBearerCtxtReq,
                                      self);
    }else{
        pointer = buf;
        newNASMsg_ESM(&pointer,
                      EPSSessionManagementMessages,
                      0);
        encaps_ESM(&pointer,
                   self->current_pti,
                   ESMInformationRequest);
        emm_sendESM(self->esm->emm, buf, pointer-buf, NULL);
    }
}

gboolean ePSsession_getPCO(const EPS_Session s, gpointer pco, gsize *len){
    EPS_Session_t *self = (EPS_Session_t*)s;

    if(self->pco[0]==0x27){
        memcpy(pco, self->pco+2, self->pco[1]);
        *len = self->pco[1];
        return TRUE;
    }
    return FALSE;
}

void ePSsession_setPCO(EPS_Session s, gconstpointer pco, gsize len){
    EPS_Session_t *self = (EPS_Session_t*)s;

    self->pco[0]=0x27;
    self->pco[1]=len;
    memcpy(self->pco+2, pco, len);
}

ESM_BearerContext ePSsession_getDefaultBearer(EPS_Session s){
    EPS_Session_t *self = (EPS_Session_t*)s;
    return self->defaultBearer;
}

void ePSsession_setPDNAddress(EPS_Session s, gpointer paa, gsize len){
    EPS_Session_t *self = (EPS_Session_t*)s;
    guint8 *p;
    p = (guint8*)paa;
    self->pdn_addr_type = p[0]&0x07;
    memcpy(self->pdn_addr, p+1, len-1);
}

const char* ePSsession_getPDNAddrStr(EPS_Session s, gpointer str, gsize maxlen){
    EPS_Session_t *self = (EPS_Session_t*)s;
    struct in_addr ipv4;
    struct in6_addr ipv6;

    switch(self->pdn_addr_type) {
    case 1: /*IPv4*/
        memcpy(&(ipv4.s_addr), self->pdn_addr, 4);
        return inet_ntop(AF_INET, &ipv4, str, maxlen);
    case 2: /*IPv6*/
        memcpy(&(ipv6.s6_addr), self->pdn_addr, 16);
        return inet_ntop(AF_INET6, &ipv6, str, maxlen);
    case 3: /* IPv4v6*/
        memcpy(&(ipv4.s_addr), self->pdn_addr, 4);
        return inet_ntop(AF_INET, &ipv4, str, maxlen);
    default:
        /* strncpy(s, "Unknown AF", maxlen); */
        return "Not valid";
    }
}

const guint8 ePSsession_getPDNType(EPS_Session s){
    EPS_Session_t *self = (EPS_Session_t*)s;
    return self->pdn_addr_type;
}


void ePSsession_getPDNAddr(const EPS_Session s, TransportLayerAddress_t* addr){
    EPS_Session_t *self = (EPS_Session_t*)s;

    switch(self->pdn_addr_type) {
    case 1: /*IPv4*/
        memcpy(addr->addr, self->pdn_addr, 4);
        addr->len = 32;
        break;
    case 2: /*IPv6*/
        memcpy(addr->addr, self->pdn_addr, 16);
        addr->len = 128;
        break;
    case 3: /* IPv4v6*/
        memcpy(addr->addr, self->pdn_addr, 20);
        addr->len = 160;
        break;
    default:
        /* strncpy(s, "Unknown AF", maxlen); */
        g_error("PDN Addr not valid in EPS Session");
    }
}

void ePSsession_modifyE_RABList(EPS_Session s, E_RABsToBeModified_t* l,
                                void (*cb)(gpointer), gpointer args){
    ESM_BearerContext bearer;
    E_RABSetupItemCtxtSURes_t *item;
    struct fteid_t fteid;
    EPS_Session_t *self = (EPS_Session_t*)s;

    memset(&fteid, 0, sizeof(struct fteid_t));
    bearer = ePSsession_getDefaultBearer(self);

    item = l->item[0]->value;
    if (esm_bc_getEBI(bearer) != item->eRAB_ID.id){
        return;
    }

    memcpy(&(fteid.teid), &(item->gTP_TEID.teid), 4);
    if (item->transportLayerAddress->len == 32){
        fteid.ipv4=1;
        memcpy(&(fteid.addr.addrv4), &(item->transportLayerAddress->addr), 4);
    }else{
        log_msg(LOG_ERR, 0, "Only IPv4 implemented, len %u, %x",
                item->transportLayerAddress->len);
    }
    fteid.iface = hton8(S1U_eNB);
    esm_bc_setS1ueNBfteid(bearer, &fteid);

    S11_Attach_ModifyBearerReq(self->s11,
                               (void(*)(gpointer)) cb,
                               (gpointer)args);
}

void ePSsession_UEContextReleaseReq(EPS_Session s,
                                    void (*cb)(gpointer), gpointer args){
    EPS_Session_t *self = (EPS_Session_t*)s;

    S11_ReleaseAccessBearers(self->s11, cb, args);
}

void ePSsession_detach(EPS_Session s, void(*cb)(gpointer), gpointer args){
    EPS_Session_t *self = (EPS_Session_t*)s;
    S11_detach(self->s11, cb, args);
}

void ePSsession_errorESM(EPS_Session s){
    EPS_Session_t *self = (EPS_Session_t*)s;
    log_msg(LOG_ERR, 0, "ESM Error indication, deleting Session");
    self->esm=NULL;
    S11_detach(self->s11, NULL, NULL);
}
