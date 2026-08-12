#!/usr/bin/env -S uv run --script
import asyncio
import bleak
from bleak import BleakClient, BleakScanner, BLEDevice


EIE_SERVICE_UUID = 'dc95d873-598d-48a2-abf5-2b056080b8ed'
EIE_SERVICE_NOTIFY_CHARACTERISTIC_UUID = 'dc95d873-598d-48a2-abf5-2b056080b8ee'
EIE_SERVICE_WRITE_CHARACTERISTIC_UUID = 'dc95d873-598d-48a2-abf5-2b056080b8ef'

found_device: BLEDevice

def notify_callback(char: bleak.BleakGATTCharacteristic, data: bytearray) -> None:
    print("recieved {data}")

async def main():
    stop_event = asyncio.Event()


    def scan_callback(device, advertising_data):
        if (len(advertising_data.service_uuids )> 0
            and advertising_data.service_uuids[0] == EIE_SERVICE_UUID
            and advertising_data.local_name == "Test EIE device"):
            global found_device
            found_device = device
            print(device, advertising_data.service_uuids[0], advertising_data.local_name)
            stop_event.set()

    async with BleakScanner(scan_callback) as scanner:
        await stop_event.wait()

    async with BleakClient(found_device) as client:
        print(f"Connected to {client.address}")
        await client.start_notify(EIE_SERVICE_NOTIFY_CHARACTERISTIC_UUID, notify_callback)
        await client.write_gatt_char(EIE_SERVICE_WRITE_CHARACTERISTIC_UUID, bytearray([10, 0, 1, 2, 3, 4, 5]), False)

asyncio.run(main())
