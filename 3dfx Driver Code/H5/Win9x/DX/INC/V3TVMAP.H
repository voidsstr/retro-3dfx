
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define V3TVMAP_DEVICE_ID 0x9624

Begin_Service_Table(V3TVMAP, VxD)

Declare_Service(V3TVMAP_GetVersion, VxD_CODE)
Declare_Service(V3TVMAP_NotifyMap, VxD_CODE)
Declare_Service(V3TVMAP_QueryActive, VxD_CODE)
Declare_Service(V3TVMAP_NotifyCapture, VxD_CODE)


End_Service_Table(V3TVMAP, VxD)

#ifdef __cplusplus
}
#endif // __cplusplus
