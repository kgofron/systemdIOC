#!/bin/bash

# Script to generate service-specific .bob screens
# Usage: ./generate_service_screen.sh <service_name> <pv_prefix> <pv_suffix>

if [ $# -ne 3 ]; then
    echo "Usage: $0 <service_name> <pv_prefix> <pv_suffix>"
    echo "Example: $0 apache2.service web: server:"
    echo "This will create apache2_service.bob with PVs like web:server:Status"
    exit 1
fi

SERVICE_NAME="$1"
PV_PREFIX="$2"
PV_SUFFIX="$3"
OUTPUT_FILE="${SERVICE_NAME%.service}_service.bob"

echo "Generating screen for service: $SERVICE_NAME"
echo "PV prefix: $PV_PREFIX"
echo "PV suffix: $PV_SUFFIX"
echo "Output file: $OUTPUT_FILE"

# Create the service-specific .bob file
cat > "$OUTPUT_FILE" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<display version="2.0.0" class="org.csstudio.opibuilder.OPIRunner">
  <id>0</id>
  <name>${SERVICE_NAME%.service}_service</name>
  <x>0</x>
  <y>0</y>
  <width>800</width>
  <height>600</height>
  <widget type="Label" version="2.0.0">
    <id>1</id>
    <name>Title</name>
    <x>20</x>
    <y>20</y>
    <width>760</width>
    <height>40</height>
    <text>$SERVICE_NAME Control</text>
    <font>
      <opifont.name>Header 1</opifont.name>
      <opifont.size>18</opifont.size>
      <opifont.style>1</opifont.style>
    </font>
    <foreground_color>
      <color red="0" green="0" blue="0"/>
    </foreground_color>
    <background_color>
      <color red="240" green="240" blue="240"/>
    </background_color>
    <border_style>1</border_style>
    <border_width>2</border_width>
    <border_color>
      <color red="0" green="0" blue="0"/>
    </border_color>
    <horizontal_alignment>1</horizontal_alignment>
    <vertical_alignment>1</vertical_alignment>
  </widget>
  
  <!-- Service Status Section -->
  <widget type="Label" version="2.0.0">
    <id>4</id>
    <name>StatusLabel</name>
    <x>20</x>
    <y>80</y>
    <width>200</width>
    <height>30</height>
    <text>Status:</text>
    <font>
      <opifont.name>Header 2</opifont.name>
      <opifont.size>14</opifont.size>
      <opifont.style>1</opifont.style>
    </font>
    <foreground_color>
      <color red="0" green="0" blue="0"/>
    </foreground_color>
  </widget>
  
  <widget type="TextUpdate" version="2.0.0">
    <id>5</id>
    <name>ServiceStatus</name>
    <x>220</x>
    <y>80</y>
    <width>200</width>
    <height>30</height>
    <pv_name>$PV_PREFIX$PV_SUFFIX Status</pv_name>
    <font>
      <opifont.name>Header 2</opifont.name>
      <opifont.size>14</opifont.size>
      <opifont.style>0</opifont.style>
    </font>
    <foreground_color>
      <color red="0" green="0" blue="0"/>
    </foreground_color>
    <background_color>
      <color red="255" green="255" blue="255"/>
    </background_color>
    <border_style>1</border_style>
    <border_width>1</border_width>
    <border_color>
      <color red="128" green="128" blue="128"/>
    </border_color>
    <horizontal_alignment>1</horizontal_alignment>
    <vertical_alignment>1</vertical_alignment>
    <rules>
      <rule name="StatusColor" prop_id="foreground_color">
        <exp bool_exp="pv[0]==&quot;running&quot;">
          <value>
            <color red="0" green="128" blue="0"/>
          </value>
        </exp>
        <exp bool_exp="pv[0]==&quot;stopped&quot;">
          <value>
            <color red="255" green="0" blue="0"/>
          </value>
        </exp>
        <exp bool_exp="pv[0]==&quot;starting&quot;">
          <value>
            <color red="255" green="165" blue="0"/>
          </value>
        </exp>
        <exp bool_exp="pv[0]==&quot;stopping&quot;">
          <value>
            <color red="255" green="165" blue="0"/>
          </value>
        </exp>
        <exp bool_exp="pv[0]==&quot;not-found&quot;">
          <value>
            <color red="128" green="128" blue="128"/>
          </value>
        </exp>
      </rule>
    </rules>
  </widget>
  
  <!-- Control Buttons Section -->
  <widget type="Label" version="2.0.0">
    <id>6</id>
    <name>ControlLabel</name>
    <x>20</x>
    <y>130</y>
    <width>200</width>
    <height>30</height>
    <text>Control:</text>
    <font>
      <opifont.name>Header 2</opifont.name>
      <opifont.size>14</opifont.size>
      <opifont.style>1</opifont.style>
    </font>
    <foreground_color>
      <color red="0" green="0" blue="0"/>
    </foreground_color>
  </widget>
  
  <!-- Start/Stop Button -->
  <widget type="ActionButton" version="2.0.0">
    <id>7</id>
    <name>StartStopButton</name>
    <x>220</x>
    <y>130</y>
    <width>120</width>
    <height>40</height>
    <text>Start</text>
    <font>
      <opifont.name>Header 2</opifont.name>
      <opifont.size>12</opifont.size>
      <opifont.style>1</opifont.style>
    </font>
    <foreground_color>
      <color red="255" green="255" blue="255"/>
    </foreground_color>
    <background_color>
      <color red="0" green="128" blue="0"/>
    </background_color>
    <border_style>1</border_style>
    <border_width>2</border_width>
    <border_color>
      <color red="0" green="0" blue="0"/>
    </border_color>
    <horizontal_alignment>1</horizontal_alignment>
    <vertical_alignment>1</vertical_alignment>
    <actions>
      <action type="WRITE_PV" version="2.0.0">
        <pv_name>$PV_PREFIX$PV_SUFFIX Start</pv_name>
        <value>1</value>
        <confirm_message>Are you sure you want to start $SERVICE_NAME?</confirm_message>
      </action>
    </actions>
    <rules>
      <rule name="ButtonText" prop_id="text">
        <exp bool_exp="pv[0]==1">
          <value>Stop</value>
        </exp>
        <exp bool_exp="pv[0]==0">
          <value>Start</value>
        </exp>
      </rule>
      <rule name="ButtonColor" prop_id="background_color">
        <exp bool_exp="pv[0]==1">
          <value>
            <color red="255" green="0" blue="0"/>
          </value>
        </exp>
        <exp bool_exp="pv[0]==0">
          <value>
            <color red="0" green="128" blue="0"/>
          </value>
        </exp>
      </rule>
      <rule name="ButtonAction" prop_id="actions">
        <exp bool_exp="pv[0]==1">
          <value>
            <action type="WRITE_PV" version="2.0.0">
              <pv_name>$PV_PREFIX$PV_SUFFIX Start</pv_name>
              <value>0</value>
              <confirm_message>Are you sure you want to stop $SERVICE_NAME?</confirm_message>
            </action>
          </value>
        </exp>
        <exp bool_exp="pv[0]==0">
          <value>
            <action type="WRITE_PV" version="2.0.0">
              <pv_name>$PV_PREFIX$PV_SUFFIX Start</pv_name>
              <value>1</value>
              <confirm_message>Are you sure you want to start $SERVICE_NAME?</confirm_message>
            </action>
          </value>
        </exp>
      </rule>
    </rules>
  </widget>
  
  <!-- Reset Failed Button -->
  <widget type="ActionButton" version="2.0.0">
    <id>8</id>
    <name>ResetFailedButton</name>
    <x>360</x>
    <y>130</y>
    <width>120</width>
    <height>40</height>
    <text>Reset Failed</text>
    <font>
      <opifont.name>Header 2</opifont.name>
      <opifont.size>12</opifont.size>
      <opifont.style>1</opifont.style>
    </font>
    <foreground_color>
      <color red="255" green="255" blue="255"/>
    </foreground_color>
    <background_color>
      <color red="255" green="165" blue="0"/>
    </background_color>
    <border_style>1</border_style>
    <border_width>2</border_width>
    <border_color>
      <color red="0" green="0" blue="0"/>
    </border_color>
    <horizontal_alignment>1</horizontal_alignment>
    <vertical_alignment>1</vertical_alignment>
    <actions>
      <action type="WRITE_PV" version="2.0.0">
        <pv_name>$PV_PREFIX$PV_SUFFIX ResetFailed</pv_name>
        <value>1</value>
        <confirm_message>Are you sure you want to reset the failed state of $SERVICE_NAME?</confirm_message>
      </action>
    </actions>
  </widget>
  
  <!-- Status Information Panel -->
  <widget type="Label" version="2.0.0">
    <id>9</id>
    <name>InfoLabel</name>
    <x>20</x>
    <y>200</y>
    <width>760</width>
    <height>30</height>
    <text>Status Information</text>
    <font>
      <opifont.name>Header 2</opifont.name>
      <opifont.size>14</opifont.size>
      <opifont.style>1</opifont.style>
    </font>
    <foreground_color>
      <color red="0" green="0" blue="0"/>
    </foreground_color>
    <background_color>
      <color red="240" green="240" blue="240"/>
    </background_color>
    <border_style>1</border_style>
    <border_width>1</border_width>
    <border_color>
      <color red="128" green="128" blue="128"/>
    </border_color>
    <horizontal_alignment>0</horizontal_alignment>
    <vertical_alignment>1</vertical_alignment>
  </widget>
  
  <!-- Status Legend -->
  <widget type="Label" version="2.0.0">
    <id>10</id>
    <name>LegendLabel</name>
    <x>20</x>
    <y>240</y>
    <width>760</width>
    <height>200</height>
    <text>Status Legend:
• running (Green): $SERVICE_NAME is active and running
• stopped (Red): $SERVICE_NAME is inactive/stopped
• starting (Orange): $SERVICE_NAME is in the process of starting
• stopping (Orange): $SERVICE_NAME is in the process of stopping
• not-found (Gray): $SERVICE_NAME is not found or not loaded
• unknown (Black): $SERVICE_NAME state is unknown

Control Instructions:
• Start/Stop Button: Click to start (green) or stop (red) $SERVICE_NAME
• Reset Failed Button: Click to reset the failed state of $SERVICE_NAME
• Status updates automatically every second
• Confirmation dialogs appear before performing actions

PV Names Used:
• Status: $PV_PREFIX$PV_SUFFIX Status
• Start/Stop: $PV_PREFIX$PV_SUFFIX Start
• Reset Failed: $PV_PREFIX$PV_SUFFIX ResetFailed</text>
    <font>
      <opifont.name>Default</opifont.name>
      <opifont.size>10</opifont.size>
      <opifont.style>0</opifont.style>
    </font>
    <foreground_color>
      <color red="0" green="0" blue="0"/>
    </foreground_color>
    <background_color>
      <color red="255" green="255" blue="255"/>
    </background_color>
    <border_style>1</border_style>
    <border_width>1</border_width>
    <border_color>
      <color red="200" green="200" blue="200"/>
    </border_color>
    <horizontal_alignment>0</horizontal_alignment>
    <vertical_alignment>0</vertical_alignment>
  </widget>
</display>
EOF

echo "Generated $OUTPUT_FILE successfully!"
echo ""
echo "To use this screen:"
echo "1. Open CSS Phoebus"
echo "2. Load the file: $OUTPUT_FILE"
echo "3. The screen will connect to PVs:"
echo "   - $PV_PREFIX$PV_SUFFIX Status"
echo "   - $PV_PREFIX$PV_SUFFIX Start"
echo "   - $PV_PREFIX$PV_SUFFIX ResetFailed"
echo ""
echo "Make sure your IOC is running with matching PV prefixes!"
