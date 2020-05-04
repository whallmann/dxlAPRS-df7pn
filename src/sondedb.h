/*
 * dxlsonde toolchain
 *
 * Copyright (C) Andreas Hummel <sonde@dotnet-dev.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef sondeaprs_H_
#include "sondeaprs.h"
#endif
 
#ifndef sondedb_H_
#define sondedb_H_

extern unsigned char sondedb_csv;
extern char sondedb_mysql_user[100];
extern char sondedb_mysql_pass[100];
extern char sondedb_mysql_db[100];
extern char sondedb_mysql_server[100];

extern void sondedb_BEGIN(void);

extern void senddata_db(sondeaprs_type, double, double, double, double, double,
                double, double, double, double, double, double, double,
                double, double, double, double, double, unsigned long,
                unsigned long, char [], unsigned long, double,
                unsigned long, char [], unsigned long, unsigned long, 
                unsigned long, char*, unsigned long, unsigned char,
                const char*, unsigned long, char*, char*, char*, char*,
		char*, double, double, char*, unsigned char,
		unsigned char, char*);

#endif /* sondedb_H_ */