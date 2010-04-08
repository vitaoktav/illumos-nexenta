/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 *
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */


#ifndef _DB_DEFS_API_
#define	_DB_DEFS_API_
#ifdef sun
#define	SIGNED
#else
#define	SIGNED signed
#endif

typedef SIGNED char		ACS;
#define	MIN_ACS		0
#define	MAX_ACS		31
#define	ANY_ACS		-1

#define	MIN_VTD		0x80

typedef SIGNED char		LSM;
#define	MIN_LSM		0
#define	MAX_LSM		126
#define	ANY_LSM		-1

typedef char		PORT;
#define	MIN_PORT		0
#define	MAX_PORT		15
#define	PORT_NAME_SIZE		70

#define	CSI_NAME_LENGTH 16
#define	MAX_CSI_LABEL		20
#define	MAX_CSI_DESCRIP 1000

typedef SIGNED char		CAP;
#define	MIN_CAP		0
#define	MAX_CAP		2
#define	ANY_CAP		-1
#define	ALL_CAP		-2

typedef char		PANEL;
#define	MIN_PANEL		0
#define	MAX_PANEL		60

typedef char		DRIVE;
#define	MIN_DRIVE		0
#define	MAX_DRIVE		19

typedef char		ROW;
#define	MIN_ROW		0
#define	MAX_ROW		41

typedef char		COL;
#define	MIN_COL		0
#define	MAX_COL		23

typedef short		LOCKID;
#define	NO_LOCK_ID		0
#define	MIN_LOCK_ID		0
#define	MAX_LOCK_ID		32767

typedef SIGNED char		MEDIA_TYPE;
#define	ANY_MEDIA_TYPE		(-1)
#define	ALL_MEDIA_TYPE		(-2)
#define	UNKNOWN_MEDIA_TYPE	(-3)

typedef SIGNED char		DRIVE_TYPE;
#define	ANY_DRIVE_TYPE		(-1)
#define	UNKNOWN_DRIVE_TYPE	(-3)

typedef long		POOL;
#define	COMMON_POOL		0
#define	MAX_POOL		65534
#define	SAME_POOL		-1

#define	MAX_WATER_MARK		2147483647
#define	MIN_WATER_MARK		0

typedef SIGNED char		CAP_PRIORITY;
#define	NO_PRIORITY		0
#define	MIN_PRIORITY		1
#define	MAX_PRIORITY		16
#define	SAME_PRIORITY		-1

typedef char		REQUEST_PRIORITY;

#define	PCAP_PRIORITY		98
#define	DEFAULT_PRIORITY 25
#define	PCAP_SIZE		1

typedef SIGNED int PANEL_TYPE;

typedef char		HAND;
#define	MIN_HAND		0
#define	MAX_HAND		1

typedef unsigned long		EVENT_SEQUENCE;

#define	EXTERNAL_USERID_SIZE		64
typedef struct {
	char user_label[EXTERNAL_USERID_SIZE];
} USERID;

#define	EXTERNAL_PASSWORD_SIZE		64
typedef struct {
	char password[EXTERNAL_PASSWORD_SIZE];
} PASSWORD;

typedef struct {
	USERID		user_id;
	PASSWORD		password;
} ACCESSID;


typedef long		FREECELLS;
#ifdef LINUX
#undef OVERFLOW
#endif

#define	OVERFLOW		1
#define	EXTERNAL_LABEL_SIZE 6
#define	SOCKET_NAME_SIZE 14

#define	JOB_NAME_SIZE		8
#define	STEP_NAME_SIZE		8
#define	DATASET_NAME_SIZE		44
#define	SUBPOOL_NAME_SIZE		13
#define	GROUPID_SIZE		8
#define	MGMT_CLAS_SIZE		8


#define	MAX_DRIVE_PANEL 4

typedef enum {
	LOCATION_FIRST = 0,
	LOCATION_CELL,
	LOCATION_DRIVE,
	LOCATION_LAST
} LOCATION;

/* state codes */
typedef enum {
	STATE_FIRST = 0,
	STATE_CANCELLED,
	STATE_DIAGNOSTIC,
	STATE_IDLE,
	STATE_IDLE_PENDING,

	STATE_OFFLINE,
	STATE_OFFLINE_PENDING,
	STATE_ONLINE,
	STATE_RECOVERY,
	STATE_RUN,

	STATE_CONNECT,
	STATE_DISCONNECT,
	STATE_DISBAND_1,
	STATE_DISBAND_2,
	STATE_DISBAND_3,
	STATE_DISBAND_4,
	STATE_JOIN_1,
	STATE_JOIN_2,
	STATE_RESIGN_1,
	STATE_RESIGN_2,

	STATE_RESIGN_3,

	STATE_LAST
} STATE;

typedef enum {
	VOLUME_TYPE_FIRST = 0,
	VOLUME_TYPE_DIAGNOSTIC,
	VOLUME_TYPE_STANDARD,
	VOLUME_TYPE_DATA,
	VOLUME_TYPE_SCRATCH,
	VOLUME_TYPE_CLEAN,
	VOLUME_TYPE_MVC,
	VOLUME_TYPE_VTV,
	VOLUME_TYPE_LAST
} VOLUME_TYPE;

typedef enum {
	LABEL_ATTRIBUTE_STANDARD = 0,
	LABEL_ATTRIBUTE_VIRTUAL,
	LABEL_ATTRIBUTE_LAST
} LABEL_ATTR;

typedef enum {
	CAP_MODE_FIRST = 0,
	CAP_MODE_AUTOMATIC,
	CAP_MODE_MANUAL,
	CAP_MODE_SAME,
	CAP_MODE_LAST
} CAP_MODE;

typedef enum {
	GROUP_TYPE_FIRST = 0,
	GROUP_TYPE_VTSS,
	GROUP_TYPE_LAST
} GROUP_TYPE;
typedef enum {
	STATUS_SUCCESS = 0,
	STATUS_ACS_FULL,
	STATUS_ACS_NOT_IN_LIBRARY,
	STATUS_ACS_OFFLINE,
	STATUS_ACSLM_IDLE,

	STATUS_ACTIVITY_END,
	STATUS_ACTIVITY_START,
	STATUS_AUDIT_ACTIVITY,
	STATUS_AUDIT_IN_PROGRESS,
	STATUS_CANCELLED,

	STATUS_CAP_AVAILABLE,
	STATUS_CAP_FULL,
	STATUS_CAP_IN_USE,
	STATUS_CELL_EMPTY,
	STATUS_CELL_FULL,

	STATUS_CELL_INACCESSIBLE,
	STATUS_CELL_RESERVED,
	STATUS_CLEAN_DRIVE,
	STATUS_COMMUNICATION_FAILED,
	STATUS_CONFIGURATION_ERROR,

	STATUS_COUNT_TOO_SMALL,
	STATUS_COUNT_TOO_LARGE,
	STATUS_CURRENT,
	STATUS_DATABASE_ERROR,
	STATUS_DEGRADED_MODE,

	STATUS_DONE,
	STATUS_DOOR_CLOSED,
	STATUS_DOOR_OPENED,
	STATUS_DRIVE_AVAILABLE,
	STATUS_DRIVE_IN_USE,

	STATUS_DRIVE_NOT_IN_LIBRARY,
	STATUS_DRIVE_OFFLINE,
	STATUS_DRIVE_RESERVED,
	STATUS_DUPLICATE_LABEL,
	STATUS_EJECT_ACTIVITY,

	STATUS_ENTER_ACTIVITY,
	STATUS_EVENT_LOG_FULL,
	STATUS_IDLE_PENDING,
	STATUS_INPUT_CARTRIDGES,
	STATUS_INVALID_ACS,

	STATUS_INVALID_COLUMN,
	STATUS_INVALID_COMMAND,
	STATUS_INVALID_DRIVE,
	STATUS_INVALID_LSM,
	STATUS_INVALID_MESSAGE,

	STATUS_INVALID_OPTION,
	STATUS_INVALID_PANEL,
	STATUS_INVALID_PORT,
	STATUS_INVALID_ROW,
	STATUS_INVALID_STATE,

	STATUS_INVALID_SUBPANEL,
	STATUS_INVALID_TYPE,
	STATUS_INVALID_VALUE,
	STATUS_INVALID_VOLUME,
	STATUS_IPC_FAILURE,

	STATUS_LIBRARY_BUSY,
	STATUS_LIBRARY_FAILURE,
	STATUS_LIBRARY_NOT_AVAILABLE,
	STATUS_LOCATION_OCCUPIED,
	STATUS_LSM_FULL,

	STATUS_LSM_NOT_IN_LIBRARY,
	STATUS_LSM_OFFLINE,
	STATUS_MESSAGE_NOT_FOUND,
	STATUS_MESSAGE_TOO_LARGE,
	STATUS_MESSAGE_TOO_SMALL,

	STATUS_MISPLACED_TAPE,
	STATUS_MULTI_ACS_AUDIT,
	STATUS_NORMAL,
	STATUS_NONE,
	STATUS_NOT_IN_SAME_ACS,

	STATUS_ONLINE,
	STATUS_OFFLINE,
	STATUS_PENDING,
	STATUS_PORT_NOT_IN_LIBRARY,
	STATUS_PROCESS_FAILURE,

	STATUS_RECOVERY_COMPLETE,
	STATUS_RECOVERY_FAILED,
	STATUS_RECOVERY_INCOMPLETE,
	STATUS_RECOVERY_STARTED,
	STATUS_REMOVE_CARTRIDGES,

	STATUS_RETRY,
	STATUS_STATE_UNCHANGED,
	STATUS_TERMINATED,
	STATUS_VALID,
	STATUS_VALUE_UNCHANGED,

	STATUS_VARY_DISALLOWED,
	STATUS_VOLUME_ADDED,
	STATUS_VOLUME_EJECTED,
	STATUS_VOLUME_ENTERED,
	STATUS_VOLUME_FOUND,

	STATUS_VOLUME_HOME,
	STATUS_VOLUME_IN_DRIVE,
	STATUS_VOLUME_IN_TRANSIT,
	STATUS_VOLUME_NOT_IN_DRIVE,
	STATUS_VOLUME_NOT_IN_LIBRARY,

	STATUS_UNREADABLE_LABEL,
	STATUS_UNSUPPORTED_OPTION,
	STATUS_UNSUPPORTED_STATE,
	STATUS_UNSUPPORTED_TYPE,
	STATUS_VOLUME_IN_USE,

	STATUS_PORT_FAILURE,
	STATUS_MAX_PORTS,
	STATUS_PORT_ALREADY_OPEN,
	STATUS_QUEUE_FAILURE,
	STATUS_NI_FAILURE,
	STATUS_RPC_FAILURE = STATUS_NI_FAILURE,

	STATUS_NI_TIMEDOUT,
	STATUS_INVALID_COMM_SERVICE,
	STATUS_COMPLETE,
	STATUS_AUDIT_FAILED,
	STATUS_NO_PORTS_ONLINE,

	STATUS_CARTRIDGES_IN_CAP,
	STATUS_TRANSLATION_FAILURE,
	STATUS_DATABASE_DEADLOCK,
	STATUS_DIAGNOSTIC,
	STATUS_DUPLICATE_IDENTIFIER,

	STATUS_EVENT_LOG_FAILURE,
	STATUS_DISMOUNT_ACTIVITY,
	STATUS_MOUNT_ACTIVITY,
	STATUS_POOL_NOT_FOUND,
	STATUS_POOL_NOT_EMPTY,

	STATUS_INVALID_RANGE,
	STATUS_INVALID_POOL,
	STATUS_POOL_HIGH_WATER,
	STATUS_POOL_LOW_WATER,
	STATUS_INVALID_VERSION,

	STATUS_MISSING_OPTION,
	STATUS_INCORRECT_ATTRIBUTE,
	STATUS_INVALID_LOCKID,
	STATUS_VOLUME_AVAILABLE,
	STATUS_READABLE_LABEL,

	STATUS_NO_CAP_AVAILABLE,
	STATUS_LOCK_FAILED,
	STATUS_DEADLOCK,
	STATUS_LOCKID_NOT_FOUND,
	STATUS_INCORRECT_LOCKID,

	STATUS_SCRATCH_NOT_AVAILABLE,
	STATUS_CLEAN_DRIVE_COMPLETE,
	STATUS_VOLUME_NOT_FOUND,
	STATUS_CAP_DOOR_OPEN,
	STATUS_CAP_INOPERATIVE,

	STATUS_DISK_FULL,
	STATUS_CAP_NOT_IN_LIBRARY,
	STATUS_CAP_OFFLINE,
	STATUS_INVALID_CAP,
	STATUS_INCORRECT_CAP_MODE,

	STATUS_INCORRECT_STATE,
	STATUS_VARY_IN_PROGRESS,
	STATUS_ACS_ONLINE,
	STATUS_AUTOMATIC,
	STATUS_MANUAL,

	STATUS_VOLUME_DELETED,
	STATUS_INSERT_MAGAZINES,
	STATUS_UNSUPPORTED_COMMAND,
	STATUS_COMMAND_ACCESS_DENIED,
	STATUS_VOLUME_ACCESS_DENIED,

	STATUS_OWNER_NOT_FOUND,
	STATUS_INVALID_DRIVE_TYPE,
	STATUS_INVALID_MEDIA_TYPE,
	STATUS_INCOMPATIBLE_MEDIA_TYPE,
	STATUS_DRIVE_FOUND,

	STATUS_CAP_DONE,
	STATUS_INVALID_SUBSYSID,
	STATUS_LSM_OFFLINE_PENDING,
	STATUS_PORT_OFFLINE,
	STATUS_COMMUNICATING,

	STATUS_NOT_COMMUNICATING,
	STATUS_IDLE,
	STATUS_OFFLINE_PENDING,
	STATUS_SWITCHOVER_INITIATED,
	STATUS_SWITCHOVER_RECOVERY_COMPLETE,

	STATUS_LMU_STATUS_CHANGE_NEW_MASTER,
	STATUS_LMU_STATUS_CHANGE_STANDBY_COMM,
	STATUS_LMU_STATUS_CHANGE_STANDBY_NOT_COMM,
	STATUS_STANDBY_LMU_NO_PORTS,
	STATUS_SWITCHOVER_IN_PROGRESS,

	STATUS_LAST_MASTER_PORT,
	STATUS_NOT_CONFIGD_DUAL,
	STATUS_DISALLOWED_ON_MASTER,
	STATUS_LMU_TO_LMU_LINK_BAD,
	STATUS_TOO_MANY_NAKS,

	STATUS_MASTER_NOT_RESPONDING,
	STATUS_MOVE_ACTIVITY,
	STATUS_VOLUME_MISSING,
	STATUS_VOLUME_BEING_RECOVERED,
	STATUS_INVALID_CLIENT,

	STATUS_VOLUME_REACTIVATED,
	STATUS_VOLUME_ABSENT,
	STATUS_UNIT_ATTENTION,
	STATUS_READY,
	STATUS_NOT_READY,

	STATUS_SERIAL_NUM_CHG,
	STATUS_HARDWARE_ERROR,
	STATUS_MONITOR_COMPLETE,
	STATUS_PTP_NOT_FOUND,
	STATUS_INVALID_EVENT_CLASS,

	STATUS_HAND_NOT_FOUND,
	STATUS_LMU_NOT_FOUND,
	STATUS_OPERATIVE,
	STATUS_INOPERATIVE,
	STATUS_MAINT_REQUIRED,

	STATUS_CAP_DOOR_CLOSED,
	STATUS_LSM_TYPE_CHG,
	STATUS_CONFIGURATION_CHANGED,
	STATUS_DRIVE_TYPE_CHG,
	STATUS_NI_TIMEOUT,

	STATUS_VOLUME_OVER_MAX_CLEAN,
	STATUS_VOLUME_CLEAN_CART_SPENT,
	STATUS_DRIVE_ADDED,
	STATUS_DRIVE_REMOVED,
	STATUS_LMU_TYPE_CHG,


	STATUS_LMU_COMPAT_LVL_CHG,
	STATUS_LMU_NOW_STANDALONE,
	STATUS_LMU_NOW_MASTER,
	STATUS_LMU_NOW_STANDBY,
	STATUS_PANEL_NOT_IN_LIBRARY,


	STATUS_PANEL_NOT_FOUND,
	STATUS_STORAGE_CELL_MAP_CHANGED,
	STATUS_NOT_A_DRIVE_PANEL,
	STATUS_NO_DRIVES_FOUND,
	STATUS_LKEY_INVALID,


	STATUS_LKEY_EXPIRED,
	STATUS_LKEY_DUE_TO_EXPIRE,
	STATUS_LKEY_CAPACITY_EXCEEDED,

	STATUS_LAST
} STATUS;

typedef enum {
	MODE_FIRST = 0,
	MODE_DUAL_LMU,
	MODE_SINGLE_LMU,
	MODE_SCSI_LMU,
	MODE_NONE,
	MODE_LAST
} MODE;

typedef enum {
	ROLE_FIRST = 0,
	ROLE_MASTER_A,
	ROLE_MASTER_B,
	ROLE_STANDBY_A,
	ROLE_STANDBY_B,
	ROLE_NONE,
	ROLE_LAST
} ROLE;

typedef enum {
	CHANGE_FIRST = 0,
	CHANGE_INCREMENT,
	CHANGE_DECREMENT,
	CHANGE_LAST
} CHANGE_DIRECTION;


typedef enum {
	EVENT_CLASS_FIRST = 0,
	EVENT_CLASS_VOLUME,
	EVENT_CLASS_RESOURCE,
	EVENT_CLASS_DRIVE_ACTIVITY,
	EVENT_CLASS_LAST
} EVENT_CLASS_TYPE;

typedef enum {
	EVENT_REPLY_FIRST = 0,
	EVENT_REPLY_REGISTER,
	EVENT_REPLY_UNREGISTER,
	EVENT_REPLY_SUPERCEDED,
	EVENT_REPLY_SHUTDOWN,
	EVENT_REPLY_CLIENT_CHECK,
	EVENT_REPLY_RESOURCE,
	EVENT_REPLY_VOLUME,
	EVENT_REPLY_DRIVE_ACTIVITY,
	EVENT_REPLY_LAST
} EVENT_REPLY_TYPE;


typedef enum {
	RESOURCE_FIRST = 0,
	RESOURCE_ONLINE = 1,
	RESOURCE_OFFLINE,
	RESOURCE_OPERATIVE,
	RESOURCE_INOPERATIVE,
	RESOURCE_MAINT_REQUIRED,
	RESOURCE_UNIT_ATTENTION,
	RESOURCE_HARDWARE_ERROR,
	RESOURCE_DEGRADED_MODE,
	RESOURCE_SERIAL_NUM_CHG,
	RESOURCE_DIAGNOSTIC,
	RESOURCE_SERV_CONFIG_MISMATCH = 21,
	RESOURCE_SERV_CONFIG_CHANGE,
	RESOURCE_SERV_START,
	RESOURCE_SERV_IDLE,
	RESOURCE_SERV_IDLE_PENDING,
	RESOURCE_SERV_FAILURE,
	RESOURCE_SERV_LOG_FAILED,
	RESOURCE_SERV_LOG_FILLED,

	RESOURCE_LMU_NEW_MASTER = 31,
	RESOURCE_LMU_STBY_COMM,
	RESOURCE_LMU_STBY_NOT_COMM,
	RESOURCE_LMU_RECOVERY_COMPLETE,
	RESOURCE_LMU_NOW_STANDBY,
	RESOURCE_LMU_NOW_STANDALONE,
	RESOURCE_LMU_TYPE_CHG,
	RESOURCE_LMU_COMPAT_LVL_CHG,


	RESOURCE_CAP_DOOR_OPEN  = 41,
	RESOURCE_CAP_DOOR_CLOSED,
	RESOURCE_CARTRIDGES_IN_CAP,
	RESOURCE_CAP_ENTER_START,
	RESOURCE_CAP_ENTER_END,
	RESOURCE_CAP_REMOVE_CARTRIDGES,
	RESOURCE_NO_CAP_AVAILABLE,
	RESOURCE_CAP_INSERT_MAGAZINES,
	RESOURCE_CAP_INPUT_CARTRIDGES,

	RESOURCE_LSM_DOOR_OPENED  = 51,
	RESOURCE_LSM_DOOR_CLOSED,
	RESOURCE_LSM_RECOVERY_INCOMPLETE,
	RESOURCE_LSM_TYPE_CHG,
	RESOURCE_LSM_ADDED,
	RESOURCE_LSM_CONFIG_CHANGE,
	RESOURCE_LSM_REMOVED,

	RESOURCE_DRIVE_CLEAN_REQUEST  = 61,
	RESOURCE_DRIVE_CLEANED,
	RESOURCE_DRIVE_TYPE_CHG,
	RESOURCE_DRIVE_ADDED,
	RESOURCE_DRIVE_REMOVED,


	RESOURCE_POOL_HIGHWATER = 71,
	RESOURCE_POOL_LOWWATER,

	RESOURCE_ACS_ADDED = 81,
	RESOURCE_ACS_CONFIG_CHANGE,
	RESOURCE_ACS_REMOVED,
	RESOURCE_ACS_PORTS_CHANGE,

	RESOURCE_CAP_ADDED =  91,
	RESOURCE_CAP_CONFIG_CHANGE,
	RESOURCE_CAP_REMOVED,

	RESOURCE_LAST = 99
} RESOURCE_EVENT;

typedef struct {
	int		category;
	int		code;
} SENSE_HLI;

typedef struct {
	unsigned char		sense_key;
	unsigned char		asc;
	unsigned char		ascq;
} SENSE_SCSI;

typedef struct {
	char		fsc[4];
} SENSE_FSC;

#define	MAX_SERIAL_NUM_LENGTH		32

typedef struct {
	char		serial_nbr[MAX_SERIAL_NUM_LENGTH +1];
} SERIAL_NUM;

typedef int LSM_TYPE;

#define	RESOURCE_ALIGN_PAD_SIZE		64

#endif /* _DB_DEFS_API_ */