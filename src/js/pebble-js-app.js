function requestData(url, type, callback) {
	var xhr = new XMLHttpRequest();
	xhr.onload = function() {
		if (xhr.readyState == 4) {
			if (xhr.status == 200) {
				callback(this.responseText);
			}
			else {
				console.log("Error requesting data with URL: " + url);
			}
		}
	};

	xhr.open(type, url);
	xhr.send();
}


function locationSuccess(location) {
	var lat = location.coords.latitude;
	var lon = location.coords.longitude;

	var url = "http://api.openweathermap.org/data/2.5/weather?lat=" + lat + "&lon=" + lon;
	requestData(url, "GET", function(responseText) {
		var json = JSON.parse(responseText);

		var place = json.name;
		var temperature = json.main.temp;
		var description = json.weather[0].main;
		var pressure = json.main.pressure;
		var humidity = json.main.humidity;

		console.log("- place " + place);
		console.log("- temperature " + temperature);
		console.log("- description " + description);
		console.log("- pressure " + pressure);
		console.log("- humidity " + humidity);

		Pebble.sendAppMessage({
			"AppMessageKeyURL": url,
			"AppMessageKeyPlace": place,
			"AppMessageKeyTemperature": temperature,
			"AppMessageKeyDescription": description,
			"AppMessageKeyPressure": pressure,
			"AppMessageKeyHumidity": humidity
		},
		function(e) {
			console.log("Sent to Pebble for URL: " + url);
		},
		function(e) {
			console.log("Not sent to Pebble");
		});
	});
}


function locationError(error) {
	console.log("Location is not available");
}


function requestLocation() {
	navigator.geolocation.getCurrentPosition(locationSuccess, locationError, {
		timeout: 15000,
		maximumAge: 60000
	});
}


Pebble.addEventListener("ready", requestLocation);
Pebble.addEventListener("appmessage", requestLocation);
