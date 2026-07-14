#pragma once

#include <Windows.h>

constexpr auto XUSER_STATS_ATTRS_IN_SPEC = 1;
typedef struct _XUSER_STATS_SPEC {
	DWORD dwViewId;
	DWORD dwNumColumnIds;
	WORD rgwColumnIds[XUSER_STATS_ATTRS_IN_SPEC];
} XUSER_STATS_SPEC, *PXUSER_STATS_SPEC;

struct _XOVERLAPPED;
typedef VOID(__stdcall *XOVERLAPPED_COMPLETION_ROUTINE_t)(
	DWORD dwErrorCode,
	DWORD dwNumberOfBytesTransfered,
	struct _XOVERLAPPED* pOverlapped
);
typedef struct _XOVERLAPPED {
	ULONG_PTR InternalLow;
	ULONG_PTR InternalHigh;
	ULONG_PTR InternalContext;
	HANDLE hEvent;
	XOVERLAPPED_COMPLETION_ROUTINE_t pCompletionRoutine;
	DWORD_PTR dwCompletionContext;
	DWORD dwExtendedError;
} XOVERLAPPED, *PXOVERLAPPED;

typedef DWORD(__stdcall* XUserCreateStatsEnumeratorByRank_t)(
	DWORD dwTitleId,
	DWORD dwRankStart,
	DWORD dwNumRows,
	DWORD dwNumStatsSpecs,
	CONST XUSER_STATS_SPEC *pSpecs,
	PDWORD pcbBuffer,
	PHANDLE ph
	);

typedef DWORD(__stdcall* XEnumerate_t)(
	HANDLE hEnum,
	PVOID pvBuffer,
	DWORD cbBuffer,
	PDWORD pcItemsReturned,
	PXOVERLAPPED pOverlapped
	);

constexpr auto XUSER_DATA_TYPE_CONTEXT = 0;
constexpr auto XUSER_DATA_TYPE_INT32 = 1;
constexpr auto XUSER_DATA_TYPE_INT64 = 2;
constexpr auto XUSER_DATA_TYPE_DOUBLE = 3;
constexpr auto XUSER_DATA_TYPE_UNICODE = 4;
constexpr auto XUSER_DATA_TYPE_FLOAT = 5;
constexpr auto XUSER_DATA_TYPE_BINARY = 6;
constexpr auto XUSER_DATA_TYPE_DATETIME = 7;
constexpr auto XUSER_DATA_TYPE_NULL = 0xFF;
typedef ULONGLONG XUID;
constexpr auto XUSER_NAME_SIZE = 16;
typedef struct _XUSER_DATA {
	BYTE type;
	union {
		LONG nData;
		LONGLONG i64Data;
		double dblData;
		struct {
			DWORD cbData;
			LPWSTR pwszData;
		}string;
		FLOAT fData;
		struct {
			DWORD cbData;
			PBYTE pbData;
		}binary;
		FILETIME ftData;
	};
} XUSER_DATA, *PXUSER_DATA;
typedef struct _XUSER_STATS_COLUMN {
	WORD wColumnId;
	XUSER_DATA Value;
} XUSER_STATS_COLUMN, *PXUSER_STATS_COLUMN;
typedef struct _XUSER_STATS_ROW {
	XUID xuid;
	DWORD dwRank;
	LONGLONG i64Rating;
	CHAR szGamertag[XUSER_NAME_SIZE];
	DWORD dwNumColumns;
	PXUSER_STATS_COLUMN pColumns;
} XUSER_STATS_ROW, *PXUSER_STATS_ROW;
typedef struct _XUSER_STATS_VIEW {
	DWORD dwViewId;
	DWORD dwTotalViewRows;
	DWORD dwNumRows;
	PXUSER_STATS_ROW pRows;
} XUSER_STATS_VIEW, *PXUSER_STATS_VIEW;
typedef struct _XUSER_STATS_READ_RESULTS {
	DWORD dwNumViews;
	PXUSER_STATS_VIEW pViews;
} XUSER_STATS_READ_RESULTS, * PXUSER_STATS_READ_RESULTS;
