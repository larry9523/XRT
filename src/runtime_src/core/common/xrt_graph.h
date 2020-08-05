#ifndef core_common_graph_h
#define core_common_graph_h
typedef void * xclGraphHandle;

xclGraphHandle
xclGraphOpen(xclDeviceHandle handle, const uuid_t xclbinUUID, const char *graphName);

#endif
