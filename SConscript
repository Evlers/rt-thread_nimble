from building import *
import rtconfig

cwd = GetCurrentDir()
path = [cwd]
src = []

path += [
    cwd + '/nimble/nimble/include',
    cwd + '/nimble/nimble/host/include',
    cwd + '/nimble/nimble/host/src',
    cwd + '/nimble/nimble/host/services/ans/include',
    cwd + '/nimble/nimble/host/services/bas/include',
    cwd + '/nimble/nimble/host/services/bleuart/include',
    cwd + '/nimble/nimble/host/services/gap/include',
    cwd + '/nimble/nimble/host/services/gatt/include',
    cwd + '/nimble/nimble/host/services/ias/include',
    cwd + '/nimble/nimble/host/services/lls/include',
    cwd + '/nimble/nimble/host/services/tps/include',
    cwd + '/nimble/nimble/host/store/ram/include',
    cwd + '/nimble/nimble/host/util/include',
    cwd + '/nimble/nimble/transport/include',
    cwd + '/nimble/nimble/transport/common/hci_h4/include',
    cwd + '/porting/nimble/include',
    cwd + '/porting/npl/rtthread/include',
    cwd + '/nimble/ext/tinycrypt/include']

# Host stack
src += Glob('nimble/nimble/host/src/*.c')
# src += Glob('nimble/nimble/host/services/*/src/*.c')
src += Glob('nimble/nimble/host/services/ans/src/*.c')
src += Glob('nimble/nimble/host/services/bas/src/*.c')
src += Glob('nimble/nimble/host/services/gap/src/*.c')
src += Glob('nimble/nimble/host/services/gatt/src/*.c')
src += Glob('nimble/nimble/host/services/ias/src/*.c')
src += Glob('nimble/nimble/host/services/lls/src/*.c')
src += Glob('nimble/nimble/host/services/tps/src/*.c')
src += Glob('nimble/nimble/host/store/ram/src/*.c')
src += Glob('nimble/nimble/host/util/src/*.c')

# mesh
# if GetDepend(['RT_NIMBLE_MESH']):
#     path += [cwd + '/nimble/nimble/host/mesh/include']
#     src += Glob('nimble/nimble/host/mesh/src/*.c')

# tinycrypt
src += Glob('nimble/ext/tinycrypt/src/*.c')
ecc_dh_node = next((f for f in src if f.name == 'ecc_dh.c'), None)
if ecc_dh_node:
    src.remove(ecc_dh_node)
src += Glob('porting/ext/tinycrypt/src/ecc_dh.c')

# HCI transport
src += Glob('nimble/nimble/transport/src/*.c')
src += Glob('nimble/nimble/transport/common/hci_h4/src/*.c')

# nimble porting
src += Glob('porting/nimble/src/*.c')

# npl porting
src += Glob('porting/npl/rtthread/src/*.c')

# HCI transport porting
if GetDepend(['RT_NIMBLE_HCI_USING_RTT_UART']) and not GetDepend(['RT_NIMBLE_HCI_USING_CUSTOM_IMPL']):
    src += Glob('porting/transport/rtthread/src/ble_hci_rtthread_uart.c')
if GetDepend(['RT_NIMBLE_HCI_USING_RTT_VHCI']) and not GetDepend(['RT_NIMBLE_HCI_USING_CUSTOM_IMPL']):
    src += Glob('porting/transport/rtthread/src/ble_rtthread_vhci_dev.c')


LOCAL_CCFLAGS = ''

CPPDEFINES = ['']

if rtconfig.CROSS_TOOL == 'keil':
    LOCAL_CCFLAGS += ' --gnu --diag_suppress=111'
    # __BYTE_ORDER__ & __ORDER_BIG_ENDIAN__ & __ORDER_LITTLE_ENDIAN__ is not defined in keil, the specific values comes from gcc.
    CPPDEFINES.append('__ORDER_LITTLE_ENDIAN__=1234')
    CPPDEFINES.append('__ORDER_BIG_ENDIAN__=4321')
    CPPDEFINES.append('__BYTE_ORDER__=1234')

if rtconfig.CROSS_TOOL == 'gcc':
    LOCAL_CCFLAGS += ' -Wno-format -Wno-unused-variable -Wno-unused-but-set-variable'

# RT_USING_NIMBLE or PKG_USING_NIMBLE
group = DefineGroup('nimble', src, depend = ['RT_USING_NIMBLE'], CPPPATH = path, CPPDEFINES = CPPDEFINES, LOCAL_CCFLAGS = LOCAL_CCFLAGS)
if GetDepend(['PKG_USING_NIMBLE']):
    group = DefineGroup('nimble', src, depend = ['PKG_USING_NIMBLE'], CPPPATH = path, CPPDEFINES = CPPDEFINES, LOCAL_CCFLAGS = LOCAL_CCFLAGS)

# add nimble samples
if GetDepend(['RT_NIMBLE_USING_SAMPLES']):
    group += SConscript('samples/SConscript', exports = 'group')

Return('group')
