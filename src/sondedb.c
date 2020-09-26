/*
 * dxlsonde toolchain
 *
 * Copyright (C) Andreas Hummel <sonde@dotnet-dev.de>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
 
#include <my_global.h>
#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef sondeaprs_H_
#include "sondeaprs.h"
#endif
#ifndef sondedb_H_
#include "sondedb.h"
#endif

unsigned char sondedb_csv;
unsigned char sondedb_csv_header_printed;
char sondedb_mysql_user[100];
char sondedb_mysql_pass[100];
char sondedb_mysql_db[100];
char sondedb_mysql_server[100];

#define RAD0 1.7453292519943E-2

#define MEAS_LEN 18
#define STR_LEN  30
const char* S_NULL = "NULL";

void make_number_or_null_g(char* buffer, unsigned int buffer_len, double data, double nullIfValue)
{
    if(data!=nullIfValue)
        sprintf(buffer, "%#0.6f", data/RAD0);
    else
    {
        if(sondedb_csv)
            buffer[0] = 0;
        else
            strcpy(buffer, S_NULL);
    }
}

void make_number_or_null_d(char* buffer, unsigned int buffer_len, double data, double nullIfValue)
{
    if(data!=nullIfValue)
        sprintf(buffer, "%#0.3f", data);
    else
    {
        if(sondedb_csv)
            buffer[0] = 0;
        else
            strcpy(buffer, S_NULL);
    }
}

void make_number_or_null_i(char* buffer, unsigned int buffer_len, unsigned char data, unsigned char nullIfValue)
{
    if(data!=nullIfValue)
        sprintf(buffer, "%d", data);
    else
    {
        if(sondedb_csv)
            buffer[0] = 0;
        else
            strcpy(buffer, S_NULL);
    }
}

void make_number_or_null_ul(char* buffer, unsigned int buffer_len, unsigned long data, unsigned char nullIfValue)
{
    if(data!=nullIfValue)
        sprintf(buffer, "%d", data);
    else
    {
        if(sondedb_csv)
            buffer[0] = 0;
        else
            strcpy(buffer, S_NULL);
    }
}


void make_string_or_null(char* buffer, unsigned int buffer_len, char* data)
{
    if(data != (void*)0)
        if(sondedb_csv)
            sprintf(buffer, "%s", data);
        else
            sprintf(buffer, "'%s'", data);
    else
    {
        if(sondedb_csv)
            buffer[0] = 0;
        else
            strcpy(buffer, S_NULL);
    }
}

char* make_raw(const char* data, unsigned long len)
{
    char* ret = malloc(len*2+10);
    unsigned long i=0;
    for(i=0; i<len; i++)
        sprintf(ret+(i*2), "%02X", (unsigned char)data[i]);
    ret[i*2] = 0;
    return ret;
}

void make_number_or_null_t(char* buffer, unsigned int buffer_len, unsigned long data, unsigned long nullIfValue)
{
    if(data!=nullIfValue)
    {
        double mins = data/60.0;
        if(sondedb_csv)
            sprintf(buffer, "%02d:%02d:%02d", ((int)mins)/60, ((int)mins)%60, data%60);
        else
            sprintf(buffer, "'%02d:%02d:%02d'", ((int)mins)/60, ((int)mins)%60, data%60);
    }
    else
    {
        if(sondedb_csv)
            buffer[0] = 0;
        else
            strcpy(buffer, S_NULL);
    }
}
//---- erweitert um IP Adresse (uint32_t)
extern void senddata_db(sondeaprs_type type, double lat, double long0, double alt,
                double speed, double dir, double clb, double hp, double hyg,
                double temp, double ozon, double otemp, double pumpmA,
                double pumpv, double dewp, double mhz, double hrms,
                double vrms, unsigned long sattime, unsigned long uptime,
                char objname[], unsigned long objname_len,
                double egmaltitude, unsigned long goodsats,
                char usercall[], unsigned long usercall_len,
                unsigned long calperc, unsigned long sd_log_freq, char* sd_type, 
                unsigned long killTimer, unsigned char burstKill, const char* sd_raw,
        unsigned long sd_raw_len, char* hwType, char* hwRev, char* hwSN, 
        char* presSN, char* fwVersion, double voltage, double tempInt, 
        char* flightState, unsigned char heating, unsigned char power, char* error, unsigned long ip)
{
	
	if(sondeaprs_verb)
                {
                    printf("Entry: senddata_db\n");
                }
    // TODO: Store RAW data for all frames!
    if(type==type_send_raw_only)
        return;
    
    // DFM workaround
    //if(lat < 0.01 || long0 < 0.01 || lat > 1000 || long0 > 1000)
    //    return;
    //  sd_almanachage ist namentlich falsch, kann aber nicht geändert werden.  korrekt wäre sd_egmaltitude
	//  Jan meint Sept.2020: .. ändere sd_almanachage nach sd_egmaltitude: DONE im INSERT String geändert
	//  erweitert up IP Adresse (am Ende)
    const char* INSERT = "INSERT INTO sondedata (sd_log_time, sd_name, sd_log_freq, sd_type, sd_lat, sd_long, sd_alt, sd_speed, sd_dir, sd_clb, "
            "sd_press, sd_hyg, sd_temp, sd_ozone_val, sd_ozone_temp, sd_ozone_pump_curr, sd_ozone_pump_volt, sd_dewp, sd_freq, "
            "sd_hrms, sd_vrms, sd_sat_time, sd_uptime, sd_egmaltitude, sd_goodsats, sd_cal_perc, sd_kill_timer, sd_burstkill, sd_user, sd_raw, " 
            "sd_hw_type, sd_hw_rev, sd_hw_sn, sd_pres_sn, sd_fw_version, sd_bat_voltage, sd_temp_int, sd_flight_state, sd_heating, sd_power, sd_error, sd_ip) VALUES";

    char sd_name[objname_len+1];
    char sd_user[usercall_len+4];
    
    // Floats
    char sd_lat[MEAS_LEN];
    char sd_lon[MEAS_LEN];
    char sd_alt[MEAS_LEN];
    char sd_speed[MEAS_LEN];
    char sd_dir[MEAS_LEN];
    char sd_clb[MEAS_LEN];
    char sd_press[MEAS_LEN];
    char sd_hyg[MEAS_LEN];
    char sd_temp[MEAS_LEN];
    char sd_ozone_val[MEAS_LEN];
    char sd_ozone_temp[MEAS_LEN];
    char sd_ozone_pump_curr[MEAS_LEN];
    char sd_ozone_pump_volt[MEAS_LEN];
    char sd_dewp[MEAS_LEN];
    char sd_freq[MEAS_LEN];
    char sd_hrms[MEAS_LEN];
    char sd_vrms[MEAS_LEN];
    char sd_bat_voltage[MEAS_LEN];
    char sd_temp_int[MEAS_LEN];
    char sd_almanachage[MEAS_LEN];  // ist in wirklichkeit die egmaltitude (hoehe über NN)
    
    // Ints
    char sd_heating[MEAS_LEN];
    char sd_power[MEAS_LEN];
    
    // Strings
    char sd_hw_type[STR_LEN];
    char sd_hw_rev[STR_LEN];
    char sd_hw_sn[STR_LEN];
    char sd_pres_sn[STR_LEN];
    char sd_fw_version[STR_LEN];
    char sd_flight_state[STR_LEN];
    char sd_error[STR_LEN];
    
    // Time
    char sd_sat_time[MEAS_LEN];
    char sd_uptime[MEAS_LEN];
    char sd_kill_timer[MEAS_LEN];
	
	// LongInt
	char sd_ip[MEAS_LEN];
    
    // Create Name
    memcpy(sd_name, objname, objname_len);
    sd_name[objname_len] = 0;
    
    // Create User
    if(strlen(usercall)==0)
    {
        if(sondedb_csv)
            sd_user[0] = 0;
        else
            strcpy(sd_user, S_NULL);
    }
    else
    {
        if(sondedb_csv)
            sprintf(sd_user,"%s",usercall);
        else
            sprintf(sd_user,"'%s'",usercall);
    }
    
    make_number_or_null_g(sd_lat, MEAS_LEN, lat, 0.0);
    make_number_or_null_g(sd_lon, MEAS_LEN, long0, 0.0);
    
    make_number_or_null_d(sd_alt, MEAS_LEN, alt, 0.0);
    make_number_or_null_d(sd_speed, MEAS_LEN, speed, 0.0);
    make_number_or_null_d(sd_dir, MEAS_LEN, dir, 0.0);
    make_number_or_null_d(sd_clb, MEAS_LEN, clb, 0.0);
    make_number_or_null_d(sd_press, MEAS_LEN, hp, 0.0);
    make_number_or_null_d(sd_hyg, MEAS_LEN, hyg, 0.0);
    make_number_or_null_d(sd_temp, MEAS_LEN, temp, (double)X2C_max_real);
    make_number_or_null_d(sd_bat_voltage, MEAS_LEN, voltage, 0.0);
    make_number_or_null_d(sd_temp_int, MEAS_LEN, tempInt, 0.0);
    make_number_or_null_d(sd_almanachage, MEAS_LEN, egmaltitude, 0.0);
    
    make_number_or_null_i(sd_heating, MEAS_LEN, heating, 0);
	make_number_or_null_i(sd_power, MEAS_LEN, power, 0);
	make_number_or_null_ul(sd_ip, MEAS_LEN, ip, 0);

    make_string_or_null(sd_hw_type, STR_LEN, hwType);
    make_string_or_null(sd_hw_rev, STR_LEN, hwRev);
    make_string_or_null(sd_hw_sn, STR_LEN, hwSN);
    make_string_or_null(sd_pres_sn, STR_LEN, presSN);
    make_string_or_null(sd_fw_version, STR_LEN, fwVersion);
    make_string_or_null(sd_flight_state, STR_LEN, flightState);
    make_string_or_null(sd_error, STR_LEN, error);
    
    if(ozon != 0.0 || otemp != 0.0)
    {
        make_number_or_null_d(sd_ozone_val, MEAS_LEN, ozon, 0.0);
        make_number_or_null_d(sd_ozone_temp, MEAS_LEN, otemp, 0.0);
        make_number_or_null_d(sd_ozone_pump_curr, MEAS_LEN, pumpmA, 0.0);
        make_number_or_null_d(sd_ozone_pump_volt, MEAS_LEN, pumpv, 0.0);
    }
    else
    {
        if(sondedb_csv)
        {
            sd_ozone_val[0] = 0;
            sd_ozone_temp[0] = 0;
            sd_ozone_pump_curr[0] = 0;
            sd_ozone_pump_volt[0] = 0;
        }
        else
        {
            strcpy(sd_ozone_val, S_NULL);
            strcpy(sd_ozone_temp, S_NULL);
            strcpy(sd_ozone_pump_curr, S_NULL);
            strcpy(sd_ozone_pump_volt, S_NULL);
        }
    }
    make_number_or_null_d(sd_dewp, MEAS_LEN, dewp, (double)X2C_max_real);
    make_number_or_null_d(sd_freq, MEAS_LEN, mhz, 0.0);
    make_number_or_null_d(sd_hrms, MEAS_LEN, hrms, 0.0);
    make_number_or_null_d(sd_vrms, MEAS_LEN, vrms, 0.0);
    
    make_number_or_null_t(sd_sat_time, MEAS_LEN, sattime, 86401);
    make_number_or_null_t(sd_uptime, MEAS_LEN, uptime, 86401);
    make_number_or_null_t(sd_kill_timer, MEAS_LEN, killTimer==0xFFFF ? 0 : killTimer, 0);
    
    if(sondedb_csv && type == type_send_pos)
    {
        if(!sondedb_csv_header_printed)
        {
            printf("Log_Time;Name;Log_Freq;Type;Lat;Long;Alt;Speed;Dir;Clb;Press;Hyg;Temp;Ozone_Val;Ozone_Temp;Ozone_Pump_Curr;Ozone_Pump_Volt;Dewp;"
            "Freq;Hrms;Vrms;Sat_Time;Uptime;EGMaltitude;Goodsats;Cal_Perc;Kill_Timer;Burstkill;User;Hw_Type;Hw_Rev;Hw_SN;Pres_SN;Fw_Version;Bat_Voltage;Int_Temp;Flight_State;Heating;Power;Error\n");
            sondedb_csv_header_printed = 1;
        }
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        char NOW[64];
        strftime(NOW, sizeof(NOW), "%F %T", &tm);
        
        printf("%s;%s;%d;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%d;%d;%s;%d;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s\n",NOW,
            sd_name, sd_log_freq, sd_type, sd_lat, sd_lon, sd_alt, sd_speed, sd_dir, sd_clb, sd_press, sd_hyg, sd_temp,
            sd_ozone_val, sd_ozone_temp, sd_ozone_pump_curr, sd_ozone_pump_volt, sd_dewp, sd_freq, sd_hrms, sd_vrms,
            sd_sat_time, sd_uptime, sd_almanachage, goodsats, calperc, sd_kill_timer, burstKill, sd_user, sd_hw_type,
            sd_hw_rev, sd_hw_sn, sd_pres_sn, sd_fw_version, sd_bat_voltage, sd_temp_int, sd_flight_state, sd_heating, sd_power, sd_error);
    }
    else
    {
        // Write to DB
        MYSQL *con = mysql_init(NULL);
        if (con == NULL) 
        {
            fprintf(stderr, "%s\n", mysql_error(con));
        }
        else
        {
            if (mysql_real_connect(con, sondedb_mysql_server, sondedb_mysql_user, sondedb_mysql_pass, sondedb_mysql_db, 0, NULL, 0) == NULL)
            {
                fprintf(stderr, "%s\n", mysql_error(con));
            }
            else
            {
                char* buffer = malloc(3000);
                char* rawData = make_raw(sd_raw, sd_raw_len);
                char* pre_id = malloc(20);
                pre_id[0] = 0;
                
				/* uptime als unsigned long nutzen, sd_uptime ist ein string */
                if(uptime > 0)
                    sprintf(buffer, "SELECT getAlreadyStoredUptime('%s', %s, NOW())", sd_name, sd_uptime);
                else
			      // sd_.. Variablen sind oben schon alle als String formatiert worden
                    sprintf(buffer, "SELECT getAlreadyStoredByPos('%s', %s, %s, %s, NOW())", sd_name, sd_lat, sd_lon, sd_alt );
                
                // Look for id
                if(sondeaprs_verb)
                {
                    printf(buffer);
                    printf("\n");
                }
                
                if (mysql_query(con, buffer)) 
                    fprintf(stderr, "%s\n", mysql_error(con));
                else
                {
                    MYSQL_RES *result = mysql_store_result(con);
                    if (result)
                    {
                        MYSQL_ROW row;
                        unsigned int num_fields = mysql_num_fields(result);
                        while ((row = mysql_fetch_row(result)))
                        {
                            if(num_fields==1)
                            {
                                if(row[0])
                                    strncpy(pre_id, row[0], 19);
                            }
                            if(sondeaprs_verb)
                                printf("Pre-ID: %s\n", pre_id);
                        }
                        mysql_free_result(result);
                    }
                    else
                    {
                        fprintf(stderr, "Couldn't get results set: %s\n", mysql_error(con));
                    }
                    //
                }
                
                // Do actual insert
                if(strlen(pre_id)==0 /* && type != type_send_raw_only*/)
                {
					if( sd_log_freq < 100000000) sd_log_freq = sd_log_freq * 10; 
					
                    sprintf(buffer, "%s (NOW(), '%s', %lu, '%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %d, %d, %s, %d, %s, 0x%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)",
                        INSERT, sd_name, sd_log_freq, sd_type, sd_lat, sd_lon, sd_alt, sd_speed, sd_dir, sd_clb, sd_press, sd_hyg, sd_temp,
                        sd_ozone_val, sd_ozone_temp, sd_ozone_pump_curr, sd_ozone_pump_volt, sd_dewp, sd_freq, sd_hrms, sd_vrms,
                        sd_sat_time, sd_uptime, sd_almanachage, goodsats, calperc, sd_kill_timer, burstKill, sd_user, rawData, sd_hw_type,
                        sd_hw_rev, sd_hw_sn, sd_pres_sn, sd_fw_version, sd_bat_voltage, sd_temp_int, sd_flight_state, sd_heating, sd_power, sd_error, sd_ip);
                }
                else
                {
                    /*if(type != type_send_raw_only)*/
                    sprintf(buffer,"INSERT INTO sondedata_recv (sr_sd_id, sr_user, sr_log_time) VALUES (%s, %s, NOW())", pre_id, sd_user);
                    /*else
                        insert raw!*/
                }
                
                // Display query
                if(sondeaprs_verb)
                {
                    //printf("■<1> sd_log_freq with 'ul' format \n");
                    printf(buffer);
                    printf("\n");
                }
                if (mysql_query(con, buffer)) 
                    fprintf(stderr, "%s\n", mysql_error(con));
                else
                {
                    if(sondeaprs_verb) printf("Added row successful!\n");
                }
                
                free(rawData);
                free(buffer);
                free(pre_id);
            }
            mysql_close(con);
        }
    }
    
}

extern void sondedb_BEGIN(void)
{
    sondedb_csv = 0;
    sondedb_csv_header_printed = 0;
}
