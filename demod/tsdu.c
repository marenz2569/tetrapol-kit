#include "misc.h"
#include "tsdu.h"
#include "radio.h"
#include "misc.h"

#include <stdio.h>
#include <stdint.h>

void decode_cell_id(int cell_id) {

	int cell_id_format, cell_id_bs_id, cell_id_rsw_id;

	cell_id_format=(cell_id & 0xc00) >> 10;
	if (cell_id_format==0) {
		cell_id_bs_id=(cell_id & 0x3f0) >> 4;
		cell_id_rsw_id=(cell_id & 0x0f);
	} else if (cell_id_format==1) {
		cell_id_rsw_id=(cell_id & 0x3f0) >> 4;
		cell_id_bs_id=(cell_id & 0x0f);
	} else
		printf("Bad CELL_ID\n");

	printf("\t\tCELL_ID FORMAT=%i BS_ID=%i RSW_ID=%i\n", cell_id_format, cell_id_bs_id, cell_id_rsw_id);

}

void decode_key_reference(int key_reference) {
	int key_type, key_index;

	key_type=(key_reference & 0xf0) >>4;
	key_index=(key_reference & 0x0f);

	printf("KEY_REFERENCE=%i KEY_TYPE=%i KEY_INDEX=%i", key_reference, key_type, key_index);
	if (key_index==0)
		printf("CLEAR CALL!!! ");
// TODO: Decode key_type
}

void d_system_info(char *t) {

	int mode, bch, roam, exp;
	int ecch, atta, mux_type, sim, dc;
	int country_code;
	int network, version;
	int loc_area_id_mode, loc_id;
	int bn_id;
	int cell_id;
	int cell_bn;
	int u_ch_scrambling;
	int system_time;
	int superframe_cpt;
	int tx_max, radio_link_timeout, pwr_tx_adjust, rxlev_access;
	int min_reg_class, min_service_class;

	mode=bits_to_int(t+8, 3);
	bch=t[11];
	roam=t[14];
	exp=t[15];

	ecch=t[16];
	atta=t[17];
	mux_type=bits_to_int(t+19, 3);
	sim=t[22];
	dc=t[23];

	country_code=bits_to_int(t+24, 8);

	network=bits_to_int(t+32, 4);
	version=bits_to_int(t+36, 4);

	loc_area_id_mode=bits_to_int(t+40, 2);
	loc_id=bits_to_int(t+42, 6);

	bn_id=bits_to_int(t+48, 8);

	cell_id=bits_to_int(t+56,12);

	cell_bn=bits_to_int(t+68, 12);

	u_ch_scrambling=bits_to_int(t+81, 7);

	// radio param
	tx_max=bits_to_int(t+88, 3);
	radio_link_timeout=bits_to_int(t+91, 5);
	pwr_tx_adjust=radio_link_timeout = -76+4*bits_to_int(t+96, 4);
	rxlev_access = -92 + 4*bits_to_int(t+100, 4);

	system_time=bits_to_int(t+104, 8);

	// cell access
	min_reg_class=bits_to_int(t+112, 4);
	min_service_class=bits_to_int(t+116, 4);

	superframe_cpt=bits_to_int(t+124, 12);

	if (bch==0)
		mod_set(3);
	

	printf("\t\tCELL_STATE\n");
	printf("\t\t\tMODE=%i%i%i ", t[48], t[49], t[50]);
	switch (mode) {
		case 0:
			printf("normal\n");
			break;
		case 1:
			printf("inter BN disconnected mode\n");
			break;
		case 2:
			printf("main switch disconnected mode\n");
			break;
		case 3:
			printf("radioswitch disconnected mode\n");
			break;
		case 4:
			printf("BSC-disconnected mode\n");
			break;
		default:
			printf("reserved\n");
			break;
	}

	printf("\t\t\tBCH=%i ", bch);
	if (bch==0)
		printf("first BCH block superframe\n");
	else	
		printf("other block\n");

	printf("\t\t\tROAM=%i ", roam);
	if (roam==0)
		printf("ll RT accepted\n");
	else	
		printf("Home RT only\n");

	printf("\t\t\tEXP=%i ", exp);
	if (exp==0)
		printf("normal cell\n");
	else	
		printf("experimental cell\n");
	
	printf("\t\tCELL_CONFIG\n");
	printf("\t\t\tECCH=%i ", ecch);
	if (ecch==0)
		printf("No ECCH in service\n");
	else	
		printf("At least one ECCH in service\n");
	
	printf("\t\t\tATTA=%i ", atta);
	if (atta==0)
		printf("Attach/detach function not supported\n");
	else	
		printf("Attach/detach function supported\n");
	
	printf("\t\t\tMUX_TYPE=%i ", mux_type);
	if (mux_type==0)
		printf("TETRAPOL default type\n");
	else if (mux_type==1)	
		printf("type 2\n");
	else
		printf("reserved\n");
	
	printf("\t\t\tSIM=%i ", sim);
	if (sim==0)
		printf("Single base station cell\n");
	else	
		printf("Simulcast cell\n");
	
	printf("\t\t\tDC=%i ", dc);
	if (dc==0)
		printf("Single cell\n");
	else	
		printf("Double coverage cell\n");
	
	printf("\t\tCOUNTRY_CODE=%i\n", country_code);

	printf("\t\tSYSTEM_ID\n");
	printf("\t\t\tNETWORK=%i\n", network);
	printf("\t\t\tVERSION=%i\n", version);

	printf("\t\tLOC_AREA_ID\n");
	printf("\t\t\tMODE=%i ", loc_area_id_mode);
	switch (loc_area_id_mode) {
		case 0:
			printf("LAI = RSW_ID + BS_ID\n");
			break;
		case 1:
			printf("LAI = BS_ID\n");
			break;
		case 2:
			printf("LAI = LOC_ID\n");
			break;
		case 3:
			printf("reserved\n");
			break;
	}
	printf("\t\t\tLOC_ID=%i\n", loc_id);

	printf("\t\tBN_ID=%i\n", bn_id);

	decode_cell_id(cell_id);

	printf("\t\tCELL_BN=%x\n", cell_bn);

	printf("\t\tU_CH_SCRAMBLING=%i\n", u_ch_scrambling);

	printf("\t\tCELL_RADIO_PARAM\n");
	printf("\t\t\tTX_MAX=%i\n", tx_max);
	printf("\t\t\tRADIO_LINK_TIMEOUT=%i\n", radio_link_timeout);
	printf("\t\t\tPWR_TX_ADJUST= %i dBm\n", pwr_tx_adjust);
	printf("\t\t\tRXLEV_ACCESS= %i dBm\n", rxlev_access);

	printf("\t\tSYSTEM_TIME=%i\n", system_time);

	printf("\t\tCELL_ACCESS\n");
	printf("\t\t\tMIN_REG_CLASS=%i\n", min_reg_class);
	printf("\t\t\tMIN_SERVICE_CLASS=%i\n", min_service_class);

	printf("\t\tSUPERFRAME_CPT=%i\n", superframe_cpt);


}
	
void d_group_composition(char *t) {

	int og_nb;
	int i;
	int group_id[10];

	og_nb=bits_to_int(t+20, 4);
	group_id[0]=bits_to_int(t+8, 12);
	for (i=1; i<=og_nb; i++)
		group_id[i]=bits_to_int(t+i*12+12, 12);


	printf("\tCODOP=0x93 (D_GROUP_COMPOSITION)\n");
	printf("\t\tOG_NB=%i\n", og_nb);
	for (i=0; i<=og_nb; i++) {
		printf("\t\tGROUP_ID=%04i ", group_id[i]);
		if (group_id[i] < 3500)
			printf("simple OG");
		if ((group_id[i] >= 3500) && (group_id[i] <=3755))
			printf("multi OG");
		if (group_id[i] == 4095)
			printf("all OG");
		printf("\n");
	}
}

void d_group_activation(char *t) {

	int activation_mode, group_id, coverage_id, channel_id, u_ch_scrambling, d_ch_scrambling, key_reference, tti;

	activation_mode=bits_to_int(t+8,4);
	group_id=bits_to_int(t+12, 12);
	coverage_id=bits_to_int(t+24, 8);
	channel_id=bits_to_int(t+36,12);
	u_ch_scrambling=bits_to_int(t+48,8);
	u_ch_scrambling=bits_to_int(t+56,8);
	key_reference=bits_to_int(t+64,8);

	// TODO TTI

	printf("\tCODOP=0x55 (D_GROUP_ACTIVATION)\n");
	printf("\t\tACTIVATION_MODE=%i\n", activation_mode);
	printf("\t\tGROUP_ID=%i\n", group_id);
	printf("\t\tCOVERAGE_ID=%i\n", coverage_id);
	printf("\t\tCHANNEL_ID=%i\n", channel_id);
	printf("\t\tU_CH_SCRAMBLING=%i\n", u_ch_scrambling);
	printf("\t\tD_CH_SCRAMBLING=%i\n", d_ch_scrambling);
	printf("\t\t");
	decode_key_reference(key_reference);
	printf("\n");
}

void d_group_list(char *t) {

	int i;
	int reference_list, revision, csg, cso, dc;
	int index_list, index_list_mode, index_list_index;
	int type_nb, type_nb_type, type_nb_number;
	char *type_nb_start;
	int type_nb2, type_nb_type2, type_nb_number2;
	char *type_nb_start2;
	int och_coverage_id[64], och_call_priority[64], och_group_id[64], och_och_parameters[64], och_neghbouring_cell[64];

	reference_list=bits_to_int(t+8,8);
	revision=bits_to_int(t+8,3);
	csg=bits_to_int(t+13,1);
	cso=bits_to_int(t+14,1);
	dc=bits_to_int(t+15,1);

	index_list=bits_to_int(t+16,8);
	index_list_mode=bits_to_int(t+16,2);
	index_list_index=bits_to_int(t+18,6);

	type_nb_start=t+24;
	type_nb=bits_to_int(type_nb_start,8);
	type_nb_type=bits_to_int(type_nb_start,2);
	type_nb_number=bits_to_int(type_nb_start+2,6);
	for (i=0; i<type_nb_number; i++) {
		och_coverage_id[i]=bits_to_int(type_nb_start+8+i*40,8);
		och_call_priority[i]=bits_to_int(type_nb_start+16+i*40,4);
		och_group_id[i]=bits_to_int(type_nb_start+20+i*40,12);
		och_och_parameters[i]=bits_to_int(type_nb_start+32+i*40,4);
		och_neghbouring_cell[i]=bits_to_int(type_nb_start+36+i*40,12);
	}
	type_nb_start2=type_nb_start+8+type_nb_number*40;
	type_nb2=bits_to_int(type_nb_start2,8);
	type_nb_type2=bits_to_int(type_nb_start2,2);
	type_nb_number2=bits_to_int(type_nb_start2+2,6);
	
//TODO: TKG, EOG

	printf("\tCODOP=0x92 (D_GROUP_LIST)\n");
	printf("\t\tREFERENCE_LIST=%i REVISION=%i CSG=%i CSO=%i DC=%i\n", reference_list, revision, csg, cso, dc);
	printf("\t\tINDEX_LIST=%i MODE=%i INDEX=%i\n", index_list, index_list_mode, index_list_index);
	printf("\t\tTYPE_NB=%i TYPE=%i NUMBER=%i\n", type_nb, type_nb_type, type_nb_number);
	printf("\t\tOCH\n");
	for (i=0; i<type_nb_number; i++) {
		printf("\t\t\tCOVERAGE_ID=%i CALL_PRIORITY=%i GROUP_ID=%i OCH_PARAMETERS=%i NEIGBOURING_CELL=%i\n", och_coverage_id[i], och_call_priority[i], och_group_id[i], och_och_parameters[i], och_neghbouring_cell[i]);
	}
	printf("\t\tTYPE_NB=%i TYPE=%i NUMBER=%i\n", type_nb2, type_nb_type2, type_nb_number2);
	
}

void d_neighbouring_cell(char *t) {

	int i,j;
	int ccr_config, ccr_param;
	int bn_nb[16], channel_id[16], adjacent_param[16];
	int bn[16], loc[16], exp[16], rxlev_access[16];
	char *cell_id_list_start;
	int cell_id_list, cell_id_list_length;
	int cell_id[256];
	char *adjacent_bn_list_start;
	int adjacent_bn_list, adjacent_bn_list_length;
	int adjacent_bn[256];

	ccr_config=bits_to_int(t+12,4);
	ccr_param=bits_to_int(t+16,8);

	for (i=0; i<ccr_config; i++) {
		bn_nb[i]=bits_to_int(t+24+24*i,4);
		channel_id[i]=bits_to_int(t+28+24*i,12);
		adjacent_param[i]=bits_to_int(t+40+24*i,8);
		bn[i]=bits_to_int(t+40+24*i,1);
		loc[i]=bits_to_int(t+41+24*i,1);
		exp[i]=bits_to_int(t+43+24*i,1);
		rxlev_access[i]=bits_to_int(t+44+24*i,4);
	}

	cell_id_list=bits_to_int(t+24+24*ccr_config, 8);
	cell_id_list_length=bits_to_int(t+32+24*ccr_config, 8) / 2;	// 16-bit entries
	cell_id_list_start=t+40+24*ccr_config;

	cell_id_list_length = cell_id_list_length + 1; 			// FIXME: Why +1 ???

	for (i=0; i<cell_id_list_length; i++) {
		cell_id[i]=bits_to_int(cell_id_list_start+16*i,12);
	}

	adjacent_bn_list=bits_to_int(cell_id_list_start+16*cell_id_list_length, 8);
	adjacent_bn_list_length=bits_to_int(cell_id_list_start+8+16*cell_id_list_length, 8);
	adjacent_bn_list_start=cell_id_list_start+16+16*cell_id_list_length;

	adjacent_bn_list_length=adjacent_bn_list_length*8/12;		//12-bit entries

	for (i=0; i<adjacent_bn_list_length; i++) {
		adjacent_bn[i] = bits_to_int(adjacent_bn_list_start+12*i,12);
	}


	printf("\tCODOP=0x94 (D_NEIGHBOURING_CELL)\n");
	printf("\t\tCCR_CONFIG=%i\n", ccr_config);
	printf("\t\tCCR_PARAM=%i\n", ccr_param);

	for (i=0; i<ccr_config; i++) {
		printf("\t\t\tBN_NB=%i ", bn_nb[i]);
		printf("CHANNEL_ID=%i ", channel_id[i]);
		printf("ADJACENT_PARAM=%i BN=%i LOC=%i EXP=%i RXLEV_ACCESS=%i\n", adjacent_param[i], bn[i], loc[i], exp[i], rxlev_access[i]);
	}
	printf("\t\tCELL_ID_LIST=%i CELL_ID_LIST_LENGTH=%i\n", cell_id_list, cell_id_list_length);
	for (i=0; i<cell_id_list_length; i++) {
		printf("\t");
		decode_cell_id(cell_id[i]);
	}
	printf("\t\tADJACENT_BN_ID_LIST=%i ADJACENT_BN_ID_LIST_LENGTH=%i\n", adjacent_bn_list, adjacent_bn_list_length);
	for (i=0; i<adjacent_bn_list_length; i++) {
		printf("\t\t\tADJACENT_BN_ID=%x\n", adjacent_bn[i]);
	}

	
}

void d_tti_assignment(char *t) {
	printf("\tCODOP=0x?? (D_TTI_ASSIGNMENT)\n");
}


void d_call_waiting(char *t) {

	int appli_sap_id, call_priority, call_id;

	appli_sap_id=bits_to_int(t+8,4);
	call_priority=bits_to_int(t+12,4);
	call_id=bits_to_int(t+16,8);

// TODO: CALLING_ADDR

	printf("\tCODOP=0x12 (D_CALL_WAITING)\n");
	printf("\t\tAPPLI_SAP_ID=%i ",appli_sap_id);
	switch(appli_sap_id) {

		case 0:
			printf("(BROADCAST)\n");
			break;
		case 1:
			printf("(TRANSPORT PROTOCOL MANAGEMENT)\n");
			break;
		case 2:
			printf("(REGISTRATION)\n");
			break;
		case 3:
			printf("(RESERVED)\n");
			break;
		case 4:
			printf("(PRIVATE CALL)\n");
			break;
		case 5:
			printf("(GROUP COMMUNICATIONS)\n");
			break;
		case 6:
			printf("(EMERGENCY OPEN CHANNEL)\n");
			break;
		case 7:
			printf("(DATA MESSAGE)\n");
			break;
		case 8:
			printf("(RT MANAGEMENT)\n");
			break;
		case 9:
			printf("(RESERVED)\n");
			break;
		case 10:
			printf("(KEY DELIVERY)\n");
			break;
		case 11:
			printf("(DATA FLOW)\n");
			break;
		case 12:
			printf("(INFO_DELIVERY)\n");
			break;
		default:
			printf("(reserved)\n");
	}

	printf("\t\tCALL_PRIORITY=%i\n", call_priority);
	printf("\t\tCALL_ID=%i\n", call_id);
}

void d_call_alert(char *t) {
	printf("\tCODOP=0x31 (D_CALL_ALERT)\n");
}

void d_connect_cch(char *t) {
	printf("\tCODOP=0x62 (D_CONNECT_CCH)\n");
}

void d_data_end(char *t) {
	printf("\tCODOP=0x48 (D_DATA_END)\n");
}

void d_data_msg_down(char *t) {
	printf("\tCODOP=0x45 (D_DATA_MSG_DOWN)\n");
}

void d_registration_ack(char *t) {

	int complete_reg;
	int rt_min_activity;
	int rt_status, fix, pro, chg, ren, tra;
	int li, cna;
	int r1, r2, r3, f, s1, s2, i1, i2, i3;
	int rt_min_registration;
	int tlr_value;
	int rt_data_info;
	int group_id;
	int iei;
	int coverage_id;

	complete_reg=bits_to_int(t+8,8);
	rt_min_activity=bits_to_int(t+16,8);
	rt_status=bits_to_int(t+24,8);
	fix=bits_to_int(t+26,1);
	pro=bits_to_int(t+28,1);
	chg=bits_to_int(t+29,1);
	ren=bits_to_int(t+30,1);
	tra=bits_to_int(t+31,1);

	li=bits_to_int(t+32,1);
	cna=bits_to_int(t+33,3);
	if (cna==1) {
		r1=bits_to_int(t+36,4);
		r2=bits_to_int(t+40,4);
		r3=bits_to_int(t+44,4);
		f=bits_to_int(t+48,4);
		s1=bits_to_int(t+52,4);
		s2=bits_to_int(t+56,4);
		i1=bits_to_int(t+60,4);
		i2=bits_to_int(t+64,4);
		i3=bits_to_int(t+68,4);
	}

	rt_min_registration=bits_to_int(t+72,8);
	tlr_value=bits_to_int(t+80,8);
	rt_data_info=bits_to_int(t+88,8);
	group_id=bits_to_int(t+96,12);
	iei=bits_to_int(t+112,8);
	if (iei == IEI_COVERAGE_ID)
		coverage_id=bits_to_int(t+120,8);

	printf("\tCODOP=0x22 (D_REGISTRATION_ACK)\n");
	printf("\t\tCOMPLETE_REG=%i\n", complete_reg);
	printf("\t\tRT_MIN_ACTIVITY=%i\n", rt_min_activity);
	printf("\t\tRT_STATUS=%i FIX=%i PRO=%i CHG=%i REN=%i TRA=%i\n", rt_status, fix, pro, chg, ren, tra);
	printf("\t\tHOST_ADR LI=%i CNA=%i ", li, cna);
	if (cna==1) {
		printf("RFSI=%x%x%x-%x-%x%x-%x%x%x\n", r1, r2, r3, f, s1, s2, i1, i2, i3);
	} else
		print_buf(t+32,40);
	printf("\t\tRT_MIN_REGISTRATION=%i\n", rt_min_registration);
	printf("\t\tTLR_VALUE=%i\n", tlr_value);
	printf("\t\tRT_DATA_INFO=%i\n", rt_data_info);
	printf("\t\tGROUP_ID=%i\n", group_id);
	if (iei == IEI_COVERAGE_ID)
		printf("\t\tCOVERAGE_ID=%i\n", coverage_id);
	
}

void d_call_connect(char *t) {
	int call_type;
	int channel_id;
	int u_ch_scrambling;
	int d_ch_scrambling;
	int key_reference;

	call_type=bits_to_int(t+8,8);
	channel_id=bits_to_int(t+20,12);
	u_ch_scrambling=bits_to_int(t+32,8);
	d_ch_scrambling=bits_to_int(t+40,8);
	key_reference=bits_to_int(t+48,8);

	printf("\tCODOP=0x34 (D_CALL_CONNECT)\n");
	printf("\t\tCALL_TYPE=%i\n", call_type);
	printf("\t\tCHANNEL_ID=%i\n", channel_id);
	printf("\t\tU_CH_SCRAMBLING=%i\n", u_ch_scrambling);
	printf("\t\tD_CH_SCRAMBLING=%i\n", d_ch_scrambling);
	printf("\t\t");
	decode_key_reference(key_reference);
	printf("\n");
	printf("\t\tVALID_RT=\n");
	print_buf(t+56,64);
// TODO: KEY_OF_CALL
}

void d_functional_short_data(uint8_t *t) {
	printf("\tCODOP=0x42 (D_FUNCTIONAL_SHORT_DATA)\n");
}

void d_call_start(uint8_t *r) {
	printf("\tCODOP=0x3e (D_CALL_START)\n");
}

void d_registration_nak(uint8_t *t) {
	printf("\tCODOP=0x21 (D_REGISTRATION_NAK)\n");
}

void d_call_setup(uint8_t *t) {
	printf("\tCODOP=0x32 (D_CALL_SETUP)\n");
}

void d_reject(uint8_t *t) {
	printf("\tCODOP=0x08 (D_REJECT)\n");
}

void d_return(uint8_t *t) {
	printf("\tCODOP=0x10 (D_RETURN)\n");
}

void d_authentication(uint8_t *t) {
	printf("\tCODOP=0x13 (D_AUTHENTICATION)\n");
}

void d_authorisation(uint8_t *t) {
	printf("\tCODOP=0x16 (D_AUTHORISATION)\n");
}

void d_channel_init(uint8_t *t) {
	printf("\tCODOP=0x18 (D_CHANNEL_INIT)\n");
}

void d_forced_registration(uint8_t *t) {
	printf("\tCODOP=0x23 (D_FORCED_REGISTRATION)\n");
}

void d_location_activity_ack(uint8_t *t) {
	printf("\tCODOP=0x25 (D_LOCATION_ACTIVITY_ACK)\n");
}

void d_call_switch(uint8_t *t) {
	printf("\tCODOP=0x35 (D_CALL_SWITCH)\n");
}

void d_call_end(uint8_t *t) {
	printf("\tCODOP=0xe2 (D_CALL_END)\n");
}

void d_explicit_short_data(uint8_t *t) {
	printf("\tCODOP=0x46 (D_EXPLICIT_SHORT_DATA)\n");
}

void d_connect_dch(char *t) {

	int dch_low_layer;
	int channel_id;
	int u_ch_scrambling;
	int d_ch_scrambling;

	dch_low_layer=bits_to_int(t+8, 8);
	channel_id=bits_to_int(t+16, 12);
	u_ch_scrambling=bits_to_int(t+32, 8);
	u_ch_scrambling=bits_to_int(t+40, 8);

	printf("\tCODOP=0x60 (D_CONNECT_DCH)\n");
	printf("\t\tDCH_LOW_LAYER=%i\n", dch_low_layer);
	printf("\t\tCHANNEL_ID=%i\n", channel_id);
	printf("\t\tU_CH_SCRAMBLING=%i\n", u_ch_scrambling);
	printf("\t\tD_CH_SCRAMBLING=%i\n", d_ch_scrambling);
}

void d_data_authentication(uint8_t *t) {
	printf("\tCODOP=0x63 (D_DATA_AUTHENTICATION)\n");
}

void decode_bch(char *t) {

	int codop;

	printf("\tBCH\n");

	printf("\tRT_REF=");
	print_buf(t+24, 4);
	printf("\tBS_REF=");
	print_buf(t+28, 4);
	printf("\tCALL_PRIO=");
	print_buf(t+36, 4);

	codop=bits_to_int(t+40, 8);
	switch (codop) {
		case D_SYSTEM_INFO:
			printf("\tCODOP=0x%02x (D_SYSTEM_INFO)\n", codop);
			d_system_info(t+40);
			break;
		default:
			printf("\tCODOP=0x%02x (Unknown) ", codop);
			print_buf(t+40, 8);
			break;
	}

}

void decode_rch_address(char *t) {

	int a, y, x;

	a = t[0];
	y=bits_to_int(t+1, 3);
	x=bits_to_int(t+4, 12);

	if (a==0) {
		printf("ACK ");
		decode_addr(t);
	} else {
		printf("NACK ");
		if (y==4)
			printf("Noise\n");
		if (y==5)
			printf("Collision\n");
	}
}

void decode_pch(char *t) {

	printf("\tPCH\n");

	// TODO: activation bitmap

	printf("\t\t");
	decode_addr(t+64);
	printf("\t\t");
	decode_addr(t+80);
	printf("\t\t");
	decode_addr(t+96);
	printf("\t\t");
	decode_addr(t+112);
}

void decode_rch(char *t) {

	printf("\tRCH\n");
	printf("\t\tTERMINAL ADDRES 1: ");
	decode_rch_address(t);
	printf("\t\tTERMINAL ADDRES 2: ");
	decode_rch_address(t+16);
	printf("\t\tTERMINAL ADDRES 3: ");
	decode_rch_address(t+32);
}

void tsdu_process(char *t, int data_length, int mod) {

	int codop;

	printf("\tSDCH\n");

	codop=bits_to_int(t, 8);
	switch (codop) {
		case D_GROUP_COMPOSITION:
			d_group_composition(t);
			break;
		case D_GROUP_ACTIVATION:
			d_group_activation(t);
			break;
		case D_GROUP_LIST:
			d_group_list(t);
			break;
		case D_NEIGHBOURING_CELL:
			d_neighbouring_cell(t);
			break;
		case D_CALL_WAITING:
			d_call_waiting(t);
			break;
		case D_CALL_ALERT:
			d_call_alert(t);
			break;
		case D_CONNECT_CCH:
			d_connect_cch(t);
			break;
		case D_DATA_END:
			d_data_end(t);
			break;
		case D_DATA_MSG_DOWN:
			d_data_msg_down(t);
			break;
		case D_REGISTRATION_ACK:
			d_registration_ack(t);
			break;
		case D_CALL_CONNECT:
			d_call_connect(t);
			break;
		case D_FUNCTIONAL_SHORT_DATA:
			d_functional_short_data(t);
			break;
		case D_CALL_START:
			d_call_start(t);
			break;
		case D_REGISTRATION_NAK:
			d_registration_nak(t);
			break;
		case D_CALL_SETUP:
			d_call_setup(t);
			break;
		case D_DATA_AUTHENTICATION:
			d_data_authentication(t);
			break;
		case D_CONNECT_DCH:
			d_connect_dch(t);
			break;
		case D_REJECT:
			d_reject(t);
			break;
		case D_RETURN:
			d_return(t);
			break;
		case D_AUTHENTICATION:
			d_authentication(t);
			break;
		case D_AUTHORISATION:
			d_authorisation(t);
			break;
		case D_CHANNEL_INIT:
			d_channel_init(t);
			break;
		case D_FORCED_REGISTRATION:
			d_forced_registration(t);
			break;
		case D_LOCATION_ACTIVITY_ACK:
			d_location_activity_ack(t);
			break;
		case D_CALL_SWITCH:
			d_call_switch(t);
			break;
		case D_CALL_END:
			d_call_end(t);
			break;
		case D_EXPLICIT_SHORT_DATA:
			d_explicit_short_data(t);
			break;
		case 99999:
			d_tti_assignment(t);
			break;
		default:
			printf("\tCODOP=0x%02x (Unknown) ", codop);
			print_buf(t, 8);
			break;
	}

}
