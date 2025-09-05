#!/usr/bin/env python3
"""
Test script for emotion image display functionality
This script simulates different emotions to test the image display
"""

import time
import serial
import struct

class EmotionTester:
    def __init__(self, port="/dev/cu.wchusbserial56D70348951", baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.ser = None
        
    def connect(self):
        """Connect to the device"""
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=1)
            print(f"Connected to {self.port}")
            return True
        except Exception as e:
            print(f"Failed to connect: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from the device"""
        if self.ser:
            self.ser.close()
            print("Disconnected")
    
    def send_emotion_test(self, emotion):
        """Send emotion test command"""
        if not self.ser:
            print("Not connected")
            return False
        
        # Simulate emotion change by sending a test message
        # This is a simplified test - in real usage, emotions are triggered by AI responses
        test_msg = f"TEST_EMOTION:{emotion}\n"
        try:
            self.ser.write(test_msg.encode())
            print(f"Sent emotion test: {emotion}")
            return True
        except Exception as e:
            print(f"Failed to send emotion: {e}")
            return False
    
    def test_all_emotions(self):
        """Test all available emotions"""
        emotions = ["HAPPY", "SAD", "ANGRY", "SURPRISE", "THINKING", "SLEEP", "LOVE"]
        
        print("Testing emotion images...")
        print("Please observe the device display for emotion changes.")
        print("Each emotion should display for 3 seconds.")
        
        for emotion in emotions:
            print(f"\nTesting emotion: {emotion}")
            self.send_emotion_test(emotion)
            time.sleep(3)
        
        print("\nEmotion test completed!")

def main():
    tester = EmotionTester()
    
    if tester.connect():
        try:
            tester.test_all_emotions()
        finally:
            tester.disconnect()
    else:
        print("Could not connect to device. Please check:")
        print("1. Device is connected via USB")
        print("2. Correct port is being used")
        print("3. Device is powered on")
        print("4. No other application is using the serial port")

if __name__ == "__main__":
    main() 