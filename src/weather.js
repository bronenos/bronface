function requestData(url, type, callback) {
	var xhr = new XMLHttpRequest();
	xhr.onload = function() { callback(this.responseText); };
	xhr.open(type, url);
	xhr.send();
}


function locationSuccess(location) {
	var lat = location.coords.latitude;
	var lon = location.coords.longitude;

	var url = "http://api.openweathermap.org/data/2.5/weather?lat=" + lat + "&lon=" + lon;
	requestData(url, "GET", function(responseText) {
		var json = JSON.parse(responseText);

		var place = json.sys.name;
		var temperature = json.main.temp;
		var description = json.weather[0].main;
		var pressure = json.main.pressure;
		var humidity = json.main.temp;

		console.log("- place " + place);
		console.log("- temperature " + temperature);
		console.log("- description " + description);
		console.log("- pressure " + pressure);
		console.log("- humidity " + humidity);

		Pebble.sendAppMessage({
			"AppMessageKeyPlace": place,
			"AppMessageKeyTemperature": temperature,
			"AppMessageKeyDescription": description,
			"AppMessageKeyPressure": pressure,
			"AppMessageKeyHumidity": humidity
		},
		function(e) {
			console.log("- sent to Pebble");
		},
		function(e) {
			console.log("- not sent to Pebble");
		});
	});
}


function locationError(error) {
	console.log("Cannot request the location");
}


function requestLocation() {
	var options = {timeout: 15000, maximumAge: 60000};
	navigator.geolocation.getCurrentPosition(locationSuccess, locationError, options);
}


Pebble.addEventListener("ready", function(e) {
	console.log("PebbleKit: JS ready");
	requestLocation();
});

Pebble.addEventListener("appmessage", function(e) {
	console.log("PebbleKit: app message received");
	requestLocation();
});
