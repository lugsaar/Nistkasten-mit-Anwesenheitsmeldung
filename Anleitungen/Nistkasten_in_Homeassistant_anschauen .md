## So schauen wir uns die Aktivitäten in den Nistkästen in Homeassistant an


#### Einstellungen im TheThingsNetwork

Im TheThingsNetwork finden wir im seitlichen Menü unter Intergrations die Applikation MQTT. Dort wird uns die Serveradresse und der Username angezeigt. In unserem Beispiel waren dies 

    eu1.cloud.thethings.network, Port 1883 und Benutzername nesting@ttn

Mit der Taste generate new API Key erzeugen wir uns das zugehörende Passwort. Nun wechseln wir zu den Live Daten unseres Nistkastens Blaumeisen.

    applikations -> nesting-box-test -> End devices -> blaumeisen -> Live data

und klicken eine der letzten Meldungen an. Es erscheint ein detailierter Json Code. Dort finden wir device_id, application_id und tenant_id.

Um die Devices per mqtt zu abonieren ist die Syntax für den Topic 

    r3/{application id}@{tenant id}/devices, in unserem Fall also r3/nesting@ttn/devices


#### Einstellungen in Homeassistant


In Homeassistant fügen wir die Integration MQTT hinzu. MQTT neu konfigurieren verlangt die oben nachgeschauten Daten Servername, Port, Benutzername und Passwort und unter - auf einen Topic hören - den entsprechenden String.

Jetzt wird die configuration.yaml mit einem Editor manuel angepasst.

In unserer Homeassistant Installation schalten wir in der linken Menüleiste mit Klick auf den Anmeldenamen die erweiterten Optionen frei.
Nun können wir über den Add On Store ssh installieren und in das Terminal wechseln. Die configuration.yaml ist im Verzeichnis config. Wir öffnen sie mit dem Editor nano:

    nano config/configuration.yaml

und ergänzen dieses für unsere beiden Nistkästen.

```
mqtt:
  sensor:
    - name: Rotkehlchen Batterie
      state_topic: "v3/nesting@ttn/devices/rotkehlchen/up"
      value_template: '{{value_json.uplink_message.decoded_payload.batterie_status_in_volt}}'
      unit_of_measurement: "V"
    - name: Rotkehlchen Nestgewicht
      state_topic: "v3/nesting@ttn/devices/rotkehlchen/up"
      value_template: '{{value_json.uplink_message.decoded_payload.nestgewicht_in_gramm}}'
      unit_of_measurement: "g"
    - name: Blaumeisen Batterie
      state_topic: "v3/nesting@ttn/devices/blaumeisen/up"
      value_template: '{{value_json.uplink_message.decoded_payload.batterie_status_in_volt}}'
      unit_of_measurement: "V"
    - name: Blaumeisen Nestgewicht
      state_topic: "v3/nesting@ttn/devices/blaumeisen/up"
      value_template: '{{value_json.uplink_message.decoded_payload.nestgewicht_in_gramm}}'
      unit_of_measurement: "g"
```

Die configuration.yaml ist sehr kritisch bezüglich der richtigen Eingaben und Einrückungen des Codes. Unter 

    Entwicklerwerkzeuge, Reiter YAML -> Konfiguration prüfen 

müssen wir unbedingt feststellen ob wir keinen Fehler gemacht haben, denn dann würde sich Homeassistant nicht mehr starten lassen. 
Wenn alles geklappt hat werden die Nistkästen nach einem Neustart auf der Homeassistant Übersicht auf einer neuen Karte Sensor angezeigt.



