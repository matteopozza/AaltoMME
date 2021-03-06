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
 * @file   NAS_ESM.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  NAS ESM implementation
 *
 * This module implements the NAS ESM interface state machine on the MME EndPoint.
 */

#include "ESM_BearerContext.h"
#include "NAS.h"
#include "logmgr.h"
#include "ESM_FSMConfig.h"

typedef struct{
	uint8_t        ebi;
	gpointer       esm;
	ESM_State      *state;
	struct fteid_t s1u_sgw;
	struct fteid_t s1u_enb;
	struct fteid_t s5s8u_pgw; 

	uint8_t        oMsg[500];
	size_t         oLen;
}ESM_BearerContext_t;

ESM_BearerContext esm_bc_init(gpointer esm, uint8_t ebi){
	ESM_BearerContext_t *self = g_new0(ESM_BearerContext_t, 1);
	self->esm = esm;
	self->ebi = ebi;
	esmChangeState(self, Inactive);
	return self;
}

void esm_bc_free(ESM_BearerContext bc_h){
	ESM_BearerContext_t *self = (ESM_BearerContext_t*)bc_h;
	g_free(self);
}


guint8 *esm_bc_getEBIp(ESM_BearerContext bc_h){
	ESM_BearerContext_t *self = (ESM_BearerContext_t*)bc_h;
	return &(self->ebi);
}

const guint8 esm_bc_getEBI(const ESM_BearerContext bc_h){
	ESM_BearerContext_t *self = (ESM_BearerContext_t*)bc_h;
	return self->ebi;
}

void esm_bc_setState(ESM_BearerContext bc, ESM_State *s){
	ESM_BearerContext_t *self = (ESM_BearerContext_t *)bc;
	self->state = s;
}

void esm_bc_processMsg(ESM_BearerContext self, const ESM_Message_t * msg){
	switch(msg->messageType){
	/*Network Initiated*/
	case ActivateDefaultEPSBearerContextAccept:
	case ActivateDefaultEPSBearerContextReject:
	case ActivateDedicatedEPSBearerContextAccept:
	case ActivateDedicatedEPSBearerContextReject:
	case ModifyEPSBearerContextAccept:
	case ModifyEPSBearerContextReject:
	case DeactivateEPSBearerContextAccept:
		break;
	default:
		break;
	}
}

void esm_activateDefault(ESM_BearerContext bc_h){
	ESM_BearerContext_t *self = (ESM_BearerContext_t*)bc_h;
}

/* void esm_DefaultEPSBearerContextActivation(gpointer bc_h){ */
/* 	ESM_BearerContext_t *self = (ESM_BearerContext_t*)bc_h; */

/* 	uint8_t *pointer, aPN[100],pco[14]; */
/*     uint32_t len; */

    /* /\* Forge Activate Default Bearer Context Req*\/ */
    /* pointer = self->oMsg; */
    /* newNASMsg_ESM(&pointer, EPSSessionManagementMessages, 5); */
    /* encaps_ESM(&pointer, 1 ,ActivateDefaultEPSBearerContextRequest); */

    /* /\* EPS QoS *\/ */
    /* nasIe_lv_t4(&pointer, &(PDATA->user_ctx->ebearer->qos.qci), 1); */

    /* /\* Access point name*\/ */
    /* encodeAPN(aPN, PDATA->user_ctx->aPname); */
    /* nasIe_lv_t4(&pointer, aPN, strlen(PDATA->user_ctx->aPname)+1); /\* This +1 is the initial label lenght*\/ */

    /* /\* PDN address *\/ */
    /* switch(PDATA->user_ctx->pAA.type){ */
    /* case  1: /\* IPv4*\/ */
    /*     len = 5; */
    /*     break; */
    /* case 2: /\* IPv6*\/ */
    /*     len = 9; */
    /*     break; */
    /* case 3: /\*IPv4v6*\/ */
    /*     len = 13; */
    /*     break; */
    /* } */
    /* nasIe_lv_t4(&pointer, (uint8_t*)&(PDATA->user_ctx->pAA), len); */

    /* /\*Optionals *\/ */
    /* /\* Protocol Configuration Options*\/ */
    /* if(PDATA->user_ctx->pco[0]==0x27){ */
    /*     if(PROC->engine->mme->uE_DNS==0){ */
    /*         log_msg(LOG_DEBUG, 0, "Writting PCO IE. Lenght: %d, first byte %#x", PDATA->user_ctx->pco[1]+2, *(PDATA->user_ctx->pco+2)); */
    /*         nasIe_tlv_t4(&pointer, 0x27, PDATA->user_ctx->pco+2, PDATA->user_ctx->pco[1]); */
    /*     *(pointer-1-PDATA->user_ctx->pco[1]) = PDATA->user_ctx->pco[1]; */
    /*     }else{ */
    /*     pco[0]=0x80; */
    /*     pco[1]=0x80; */
    /*     pco[2]=0x21; */
    /*     pco[3]=0x0a; */
    /*     pco[4]=0x01; */
    /*     pco[5]=0x00; */
    /*     pco[6]=0x00; */
    /*     pco[7]=0x0a; */
    /*     pco[8]=0x81; */
    /*     pco[9]=0x06; */
    /*     memcpy(pco+10, &(PROC->engine->mme->uE_DNS), 4); */
    /*     nasIe_tlv_t4(&pointer, 0x27, pco, 14); */
    /*     *(pointer-15) = 14; */
    /*     } */
    /* } */
/*     self->oLen = pointer-(uint8_t*) self->oMsg; */
/* } */

void esm_bc_setS1uSGWfteid(ESM_BearerContext bc_h, gpointer fteid_h, gsize len){
    struct fteid_t* fteid = (struct fteid_t*) fteid_h;
    ESM_BearerContext_t *self = (ESM_BearerContext_t*)bc_h;

    if(fteid->iface != S1U_SGW)
        g_error("Unexpected interface type, S1U_SGW expected");

    memcpy(&(self->s1u_sgw), fteid, len);
}


void esm_bc_setS1ueNBfteid(ESM_BearerContext bc_h, gpointer fteid_h){
    struct fteid_t* fteid = (struct fteid_t*) fteid_h;
    ESM_BearerContext_t *self = (ESM_BearerContext_t*)bc_h;
    gsize len;
    if(fteid->iface !=  S1U_eNB)
        g_error("Unexpected interface type, S1U_eNB expected");
    
    len = 5;
    if(fteid->ipv4){
	    len += 4;
    }
    if(fteid->ipv6){
	    len += 16;
    }
    memcpy(&(self->s1u_enb), fteid, len);
}

void esm_bc_setS5S8uPGWfteid(ESM_BearerContext bc_h, gpointer fteid_h, gsize len){
    struct fteid_t* fteid = (struct fteid_t*) fteid_h;
    ESM_BearerContext_t *self = (ESM_BearerContext_t*)bc_h;

    if(fteid->iface != S5S8U_PGW)
        g_error("Unexpected interface type, S5S8U_PGW expected");

    memcpy(&(self->s5s8u_pgw), fteid, len);
}

void esm_bc_getS1uSGWfteid(const ESM_BearerContext bc_h, gpointer fteid_h, gsize *len){
	struct fteid_t* fteid = (struct fteid_t*) fteid_h;
    ESM_BearerContext_t *self = (ESM_BearerContext_t*)bc_h;

    *len = 5;
    if(self->s1u_sgw.ipv4){
	    *len += 4;
    }
    if(self->s1u_sgw.ipv6){
	    *len += 16;
    }
    memcpy(fteid, &(self->s1u_sgw), *len);
}

void esm_bc_getS1ueNBfteid(const ESM_BearerContext bc_h, gpointer fteid_h, gsize *len){
    struct fteid_t* fteid = (struct fteid_t*) fteid_h;
    ESM_BearerContext_t *self = (ESM_BearerContext_t*)bc_h;
    *len = 5;
    if(self->s1u_enb.ipv4){
	    *len += 4;
    }
    if(self->s1u_enb.ipv6){
	    *len += 16;
    }
    memcpy(fteid, &(self->s1u_enb), *len);
}
