#include <string>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <stdint.h>
#include <tinyb.hpp>

#include <vector>
#include <thread>
#include <atomic>
#include <csignal>

using namespace tinyb;
using namespace std;

std::atomic<bool> running(true);

void signal_handler(int signum)
{
    if (signum == SIGINT) {
        running = false;
    }
}

vector<string> list_of_tiny;

bool check_address(string s) {
	bool output = false;
	
	for (int i = 0; i < list_of_tiny.size(); i++) {
		if (s == list_of_tiny[i]) {
			return true;
		}
	} 
	
	return output;
}

vector<int> accel_data;
vector<int> gyro_data;

void split_data(unsigned char *data) {
	int tempx = 0;
	int tempy = 0;
	int tempz = 0;
	if (data[0] == 'a') {
		accel_data.clear();
		for (int i = 2; i < 7; i++) {
			tempx = tempx * 10;
			tempx += data[i] - '0';
		}
		for (int i = 8; i < 13; i++) {
			tempy = tempy * 10;
			tempy += data[i] - '0';			
		}
		for (int i = 14; i < 19; i++) {
			tempz = tempz * 10;
			tempz += data[i] - '0';			
		}
		if (data[1] == '-') {
			tempx = -1 * tempx;
		}
		if (data[7] == '-') {
			tempy = -1 * tempy;
		}
		if (data[13] == '-') {
			tempz = -1 * tempz;
		}
		accel_data.push_back(tempx);
		accel_data.push_back(tempy);
		accel_data.push_back(tempz);
		return;
	}
	else if (data[0] == 'g') {
		gyro_data.clear();
		for (int i = 2; i < 7; i++) {
			tempx = tempx * 10;
			tempx += data[i] - '0';
		}
		for (int i = 8; i < 13; i++) {
			tempy = tempy * 10;
			tempy += data[i] - '0';			
		}
		for (int i = 14; i < 19; i++) {
			tempz = tempz * 10;
			tempz += data[i] - '0';			
		}
		if (data[1] == '-') {
			tempx = -1 * tempx;
		}
		if (data[7] == '-') {
			tempy = -1 * tempy;
		}
		if (data[13] == '-') {
			tempz = -1 * tempz;
		}
		gyro_data.push_back(tempx);
		gyro_data.push_back(tempy);
		gyro_data.push_back(tempz);		
		return;
	}
	else if (data[0] == '[') {
		return;
	}
	else {
		cout << "wrong data structure" << endl;
		return;
	}
}

int main(int argc, char **argv) 
{

	BluetoothManager *manager = nullptr;
	try {
		manager = BluetoothManager::get_bluetooth_manager();
	} catch(const std::runtime_error& e) {
		cerr << "Error while initializing libtinyb: " << e.what() << endl;
		exit(1);
	}

    cout << "Initialization was succesful." << endl;
	
	/* Start the discovery of devices */
    bool ret = manager->start_discovery();
    if (!ret) {
		cout << "Failed to start Discovery" << endl;
		return 0;
	}
    cout << "Discovering BLE devices" << endl;
    
	std::unique_ptr<BluetoothAdapter> adapter = manager->get_default_adapter();
	//BluetoothDevice *ess_device = NULL;
	BluetoothDevice *sensor_tag = NULL;
	//BluetoothGattService *tinyb_service = NULL;
	
	list_of_tiny.push_back("E2:48:FF:ED:4E:3D"); 
	list_of_tiny.push_back("F2:60:3B:58:BA:EC");
	
	vector<unique_ptr<BluetoothDevice>> list_devices;
	//for (int i = 0; i < 15; i++) {
	while(1) {
//		cout << "Discovered Devices: " << endl;
		list_devices = manager->get_devices();
		
		for (auto it = list_devices.begin(); it != list_devices.end(); it++) {
			if ((*it)->get_rssi() != 0) {
				if (check_address((*it)->get_address())) {
					cout << "RSSI = " << (*it)->get_rssi() << " ";
//					cout << "Class = " << (*it)->get_class_name() << " ";
					cout << "Path = " << (*it)->get_object_path() << " ";
//					cout << "Name = " << (*it)->get_name() << " ";
					//cout << "Connected = " << (*it)->get_connected() << " ";
					cout << endl;
				}



				if ((*it)->get_address() == "E2:48:FF:ED:4E:3D") {
					cout << "E2:48:FF:ED:4E:3D address found" << endl;
					sensor_tag = (*it).release();
//					sensor_tag->connect();
				}
				
				if ((*it)->get_address() == "F2:60:3B:58:BA:EC") {
					cout << "address found" << endl;
					sensor_tag = (*it).release();
					//sensor_tag->connect();
				}
			}
		}
		
		if (sensor_tag != nullptr) {
			break;
		}
		else {
			std::this_thread::sleep_for(std::chrono::seconds(4));
			//cout << endl;
		}
	}

	if (sensor_tag == nullptr) {
		cout << "Could not find device " << "E2:48:FF:ED:4E:3D" << endl;
		return 0; 
	}
	
	sensor_tag->connect();

	/* Wait for the device to come online */
	cout << "Device Coming Online... " << endl;
	cout << "Obtaining Sensor Tag services" << endl; 
		
	std::this_thread::sleep_for(std::chrono::seconds(4));
	auto service_list = sensor_tag->get_services();
	if (!service_list.empty()) {
		cout << "Service List size: " << service_list.size() << endl;
	}
	else {
		cout << "Service List is empty" << endl;
	}
	auto uuid_list = sensor_tag->get_uuids();
	if (!uuid_list.empty()) {
		cout << "UUID LIST: " << endl;
		for (int i = 0; i < uuid_list.size(); i++) {
			cout << uuid_list[i] << endl;
		}
		cout << "END OF LIST" << endl;
	}
	else {
		cout << "UUID_LIST is empty" << endl;
	}
	/*
	auto data_list = sensor_tag->get_service_data();
	if (!data_list.empty()) {
		cout << "I HAVE DATA" << endl;
	}
	else {
		cout << "NO DATA" << endl;
	}
	*/
	std::string service_uuid("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
	cout << "Waiting for service " << service_uuid << " to be discovered" << endl;
	auto tinyb_service = sensor_tag->find(&service_uuid);
/*
	cout << "Stopping Discovery... " << endl;
	ret = manager->stop_discovery();
	cout << "Discovery Stopped" << endl;	
*/
    cout << "getting tx_uuid" << endl;
    auto tx_uuid = string("6e400003-b5a3-f393-e0a9-e50e24dcca9e");
    auto tx_service = tinyb_service->find(&tx_uuid);
       
    cout << "while loop here lmao" << endl;
    
    while(1) {
		try {
			std::vector<unsigned char> response = tx_service->read_value();
			
			unsigned char *data;
			unsigned int size = response.size();
			
			if (size <= 0) {
				cout << "FUCK THIS SHIT ITS NOT WORKING" << endl;
			}
			else {
				data = response.data();
				
				cout << "raw data = ";
				for (unsigned i = 0; i < size; i++) {
					cout << data[i];
					//cout << std::hex << static_cast<int>(data[i]);
				}			
				split_data(data);
				cout << "    converted data = ";
				for (int i = 0; i < 3; i++) {
					if (data[0] == 'a') {	
						cout << accel_data[i];
					}
					else if (data[0] == 'g') {
						cout << gyro_data[i];
					}
				}
				cout << endl;
			}
		}
		catch (exception &e) {
			cout << "ERROR" << endl;
			break;
		}
	}
    
	return 0;
}
