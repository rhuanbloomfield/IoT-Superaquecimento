#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>
#include <uri/UriBraces.h>

/* HTML */

const char index_html[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html lang="pt-br">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>ProjetoIOT</title>
	<style>
		html, body {margin: 0; padding: 0;}
		body {display: flex; flex-direction: column; justify-content: center; align-items: center; background-color: lightgray;}
		div {width: fit-content; height: fit-content; padding: 32px; background-color: white; font-size: larger; box-shadow: 0px 8px 16px black}
		button {padding: 16px; font-size: large;}
		fieldset {width: 800px; margin-top: 16px;}
		h1 {margin: 0 12px;} h2 {margin: 8px 0;}
		@media screen and (max-width: 1080px)
		{
			fieldset {width: 90%; word-wrap: break-word;}
		}
	</style>
</head>
<body>
<div>
	<fieldset>
		<legend><h1>Informações da Máquina</h1></legend>
		<span>Informações serão periodicamente enviadas pelo controlador da máquina e exibidas aqui.<br>As caixinhas de marcar indicam quais formas de desligamento estão ativas no momento.</span>
			<hr>
		<input type="checkbox" id="info_check_timer" disabled> Timer: <span id="info_timer">14:37.875</span><br>
		<input type="checkbox" id="info_check_temp" disabled> Temperatura: <span id="info_temp">76°C</span><br>
		<span>Umidade: <span id="info_hum">37%</span></span>
	</fieldset>
	<fieldset>
		<legend><h1>Configurar</h1></legend>
		<span>Aqui você configura a forma de desligamento da máquina.</span><br>
			<hr>
		<input type="checkbox" id="config_check_timer"> Desligar com timer (minutos): <input type="number" min="0" step="1" id="config_timer" placeholder="15"><br> 
		<input type="checkbox" id="config_check_temp"> Desligar com temperatura (°C): <input type="number" min="0" step="1" id="config_temp" placeholder="130"><br>

		<button id="btn_off">Forçar Desligamento</button>
		<button id="btn_on">Ligar Máquina</button>
	</fieldset>
	<fieldset>
		<legend><h1>Debug</h1></legend>
		<span>Informações relevantes para os desenvolvedores.</span>
			<hr>
		<span>/getInfo response: <span id="debug_getInfo">-</span></span><br>
		<span>/turnOff response: <span id="debug_turnOff">-</span></span><br>
		<span>/turnOn request: <span id="debug_turnOn_req">-</span></span><br>
		<span>/turnOn response: <span id="debug_turnOn">-</span></span><br>
	</fieldset>
</div>
<script>
	function isValidInt(num)
	{
		return !(Number.isNaN(num) || !Number.isFinite(num) || num > Number.MAX_SAFE_INTEGER || num !== Math.floor(num));
	}

	function formatTime(us)
	{
		const ms_num = us / 1000;
		const min = String(Math.min(99, Math.floor(ms_num / 1000 / 60))).padStart(2, "0");
		const seg = String(Math.min(59, Math.floor((ms_num / 1000) % 60))).padStart(2, "0");
		const ms = String(Math.min(999, Math.floor(ms_num % 1000))).padStart(3, "0");
		return `${min}:${seg}.${ms}`;
	}

	const $info_check_timer = document.querySelector("input#info_check_timer");
	const $info_check_temp = document.querySelector("input#info_check_temp");
	const $info_timer = document.querySelector("span#info_timer");
	const $info_temp = document.querySelector("span#info_temp");
	const $info_hum = document.querySelector("span#info_hum");

	const $debug_getInfo = document.querySelector("span#debug_getInfo");
	const $debug_turnOff = document.querySelector("span#debug_turnOff");
	const $debug_turnOn_req = document.querySelector("span#debug_turnOn_req");
	const $debug_turnOn = document.querySelector("span#debug_turnOn");

	const $config_check_timer = document.querySelector("input#config_check_timer");
	const $config_check_temp = document.querySelector("input#config_check_temp");
	const $config_timer = document.querySelector("input#config_timer");
	const $config_temp = document.querySelector("input#config_temp");

	const $btn_off = document.querySelector("button#btn_off");
	const $btn_on = document.querySelector("button#btn_on");

	async function getInfo()
	{
		const response = await fetch(location.origin + "/getInfo");
		const str = await response.text();
		$debug_getInfo.innerText = str;

		const entries = str.split(",");
		$info_check_timer.checked = Number(entries[0]) === 1;
		$info_timer.innerText = formatTime(entries[1]);
		$info_check_temp.checked = Number(entries[2]) === 1;
		$info_temp.innerText = entries[3];
		$info_hum.innerText = entries[4];
	}

	async function btnOffOnClick(event)
	{
		const response = await fetch(location.origin + "/turnOff");
		const str = await response.text();
		$debug_turnOff.innerText = str;
	}
	$btn_off.addEventListener("click", async (event) => {await btnOffOnClick(event);});

	async function btnOnOnClick(event)
	{
		const red = "box-shadow: 0 0 16px red, 0 0 16px red, 0 0 16px red, 0 0 16px red, 0 0 16px red; color: red;";

		// checagem de inputs
		if (!$config_check_timer.checked && !$config_check_temp.checked)
		{
			$config_check_timer.style.cssText = red;
			$config_check_temp.style.cssText = red;

			setTimeout(function()
			{
				$config_check_timer.style.cssText = "";
				$config_check_temp.style.cssText = "";
			}, 1500)
			return;
		}

		if ($config_check_timer.checked && (!isValidInt($config_timer.valueAsNumber) || $config_timer.valueAsNumber <= 0 || $config_timer.valueAsNumber > 2**62))
		{
			$config_timer.style.cssText = red;

			setTimeout(function()
			{
				$config_timer.style.cssText = "";
			}, 1500)
			return;
		}

		if ($config_check_temp.checked && (!isValidInt($config_temp.valueAsNumber) || $config_temp.valueAsNumber <= 0 || $config_temp.valueAsNumber > 2**15 - 1))
		{
			$config_temp.style.cssText = red;

			setTimeout(function()
			{
				$config_temp.style.cssText = "";
			}, 1500)
			return;
		}

		const request_str = location.origin + "/turnOn?config_check_timer=" + Number($config_check_timer.checked) +
		"&config_timer=" + ($config_timer.value === "" ? 0 : $config_timer.value * 60 * 1000 * 1000) +
		"&config_check_temp=" + Number($config_check_temp.checked) +
		"&config_temp=" + ($config_temp.value === "" ? 0 : $config_temp.value);
		$debug_turnOn_req.innerText = request_str.replace(/\&/g, "\n");

		const response = await fetch(request_str);
		const str = await response.text();
		$debug_turnOn.innerText = str;
	}
	$btn_on.addEventListener("click", async (event) => {await btnOnOnClick(event);});

	getInfo();
	setInterval(async () => {await getInfo();}, 3000);
</script>
</body>
</html>

)rawliteral";

const char* ssid = "NOME_DA_REDE";
const char* password = "SENHA";

/* Pinos */

const int PIN_DHT = 23;
const int PIN_BUZZER = 15;
const int PIN_RELAY = 27;
const int NOTE_As7 = 3729;
const int NOTE_F7 = 5588;
const int NOTE_DELTA = NOTE_F7 - NOTE_As7;

/* Sensor de Temperatura */

DHT dht(PIN_DHT, DHT11);
float temperature = 0.0f; // Celsius
float humidity = 0.0f;

// isso é usado como callback do tipo `esp_timer_cb_t` pra um timer, por isso o void *arg
void getDHT(void *arg = NULL)
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

esp_timer_handle_t timer_sensor;
const esp_timer_create_args_t timer_create_sensor = {getDHT, NULL, ESP_TIMER_TASK, "DHT", false};

/* Display */

LiquidCrystal_I2C lcd2(0x27, 16, 2);

void lcd2PrintTime(int64_t us)
{
  const int64_t ms = std::max((int64_t)0, us / 1000);
  lcd2.printf("%02"PRId64":%02"PRId64".%03"PRId64,
  std::min(ms/1000/60,(int64_t)99),
  std::min((ms/1000) % 60, (int64_t)59),
  std::min(ms % 1000, (int64_t)999));
}

void lcd2PrintDHT()
{
  lcd2.printf("%2.1f'C, %2.1f%%", temperature, humidity);
}

/* Status */

enum Status {STATUS_IDLE, STATUS_RUNNING};
Status STATUS = STATUS_IDLE;

/* Lógica do Programa */

int config_temp = 28; // Celsius
int64_t config_timer = 20000000; // us
int64_t timer = 0;
bool config_check_temp = true;
bool config_check_timer = false;

int64_t run_timer_start = 0;

// isso é usado como callback do tipo `esp_timer_cb_t` pra um timer, por isso o void *arg
void switchToIdle(void *arg = NULL)
{
  STATUS = STATUS_IDLE;
  digitalWrite(PIN_RELAY, LOW);
  tone(PIN_BUZZER, NOTE_As7, 1000);
  timer = 0;
  run_timer_start = 0;

  lcd2.clear();
  lcd2.print("Pronto para uso!");
  lcd2.setCursor(0, 1);
  lcd2PrintDHT();
}

void logicIdle()
{
  lcd2.setCursor(0, 1);
  lcd2PrintDHT();
}

void switchToRunning()
{
  if (!config_check_timer && !config_check_temp)
  {
    lcd2.clear();
    lcd2.print("Sem condicao");
    lcd2.setCursor(0, 1);
    lcd2.print("de desligamento.");
    delay(1500);
    switchToIdle();
    return;
  }

  if (config_check_timer && config_timer <= 0)
  {
    lcd2.clear();
    lcd2.print("Timer nao pode");
    lcd2.setCursor(0, 1);
    lcd2.print("ser zero.");
    delay(1500);
    switchToIdle();
    return;
  }

  if (config_check_temp && config_temp <= 0)
  {
    lcd2.clear();
    lcd2.print("Temp. nao pode");
    lcd2.setCursor(0, 1);
    lcd2.print("ser zero.");
    delay(1500);
    switchToIdle();
    return;
  }

  if (config_check_temp && temperature > config_temp)
  {
    lcd2.clear();
    lcd2.print("A maquina esta");
    lcd2.setCursor(0, 1);
    lcd2.print("quente demais.");
    delay(1500);
    switchToIdle();
    return;
  }

  STATUS = STATUS_RUNNING;
  digitalWrite(PIN_RELAY, HIGH);
  tone(PIN_BUZZER, NOTE_F7, 500);
  run_timer_start = esp_timer_get_time();

  lcd2.clear();
  lcd2.print("Timer: ");
}

void logicRunning()
{
  const int64_t delta = esp_timer_get_time() - run_timer_start;

  lcd2.setCursor(7, 0);
  if (config_check_timer) {timer = config_timer - delta;}
  else {timer = delta;}

  lcd2PrintTime(timer);
  Serial.printf("%02"PRId64":%02"PRId64".%03"PRId64,
  std::min(timer/1000/1000/60, (int64_t)99),
  std::min((timer/1000/1000) % 60, (int64_t)59),
  std::min((timer/1000) % 1000, (int64_t)999));
  lcd2.setCursor(0, 1);
  lcd2PrintDHT();

  Serial.printf(" // %2.1f'C, %2.1f%%", temperature, humidity);
  Serial.println();

  if ((config_check_timer && delta > config_timer) || (config_check_temp && temperature >= config_temp)) {switchToIdle();}
}

/* Web Server */

WebServer server(80);

void handleRoot()
{
  Serial.println("Request para /");
  server.send(200, "text/html", index_html);
}

void handleGetInfo()
{
  Serial.println("Request para /getInfo");
  String response = "";
  response += config_check_timer ? 1 : 0;
  response += ",";
  response += timer;
  response += ",";
  response += config_check_temp ? 1 : 0;
  response += ",";
  response += temperature;
  response += ",";
  response += humidity;
  server.send(200, "text/plain", response);
}

void handleTurnOff()
{
  Serial.println("Request para /turnOff");
  switchToIdle();
  server.send(200, "text/plain", "OK");
}

void handleTurnOn()
{
  Serial.println("Request para /turnOn");

  config_check_timer = server.arg("config_check_timer") == "1";
  Serial.println(server.arg("config_check_timer"));

  std::string str_config_timer = server.arg("config_timer").c_str();
  config_timer = std::stoll(str_config_timer);
  Serial.println(server.arg("config_timer"));

  config_check_temp = server.arg("config_check_temp") == "1";
  Serial.println(server.arg("config_check_temp"));

  std::string str_config_temp = server.arg("config_temp").c_str();
  config_temp = std::stoi(str_config_temp);
  Serial.println(server.arg("config_temp"));

  switchToRunning();

  server.send(200, "text/plain", "OK");
}

/* Setup & Loop */

void setup() {
  lcd2.init();
  lcd2.setBacklight(HIGH);
  lcd2.clear();
  lcd2.print("Inicializando...");

  Serial.begin(112500);
  Serial.println("Inicializando...");

  WiFi.begin(ssid, password);

  lcd2.clear();
  lcd2.print("Conectando-se");
  lcd2.setCursor(0, 1);
  lcd2.print("ao WiFi...");
  Serial.println("Conectando-se ao WiFi...");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  lcd2.clear();
  lcd2.print("Iniciando");
  lcd2.setCursor(0, 1);
  lcd2.print("servidor Web...");
  Serial.println("Iniciando servidor Web");

  server.on("/", handleRoot);
  server.on("/getInfo", handleGetInfo);
  server.on("/turnOff", handleTurnOff);
  server.on(UriBraces("/turnOn{}"), handleTurnOn);
  server.begin();

  lcd2.clear();
  lcd2.print("Anote o website!");
  lcd2.setCursor(0, 1);
  lcd2.print(WiFi.localIP());

  Serial.print("Conectado ao IP: ");
  Serial.println(WiFi.localIP());

  pinMode(PIN_DHT, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);

  dht.begin();
  getDHT(); // faz a temperatura e umidade não começarem em zero

  /* Timers */
  esp_timer_create(&timer_create_sensor, &timer_sensor);

  // isso vai atualizar a temperatura e a umidade periodicamente
  esp_timer_start_periodic(timer_sensor, 2000000);

  /* Finalização */
  Serial.println("Iniciado com sucesso.");
  delay(4000); // delay artificial pra dar a sensação de carregamento e dar tempo de ver o IP

  switchToIdle();
}

void loop() {
  // usado pra calcular o delay
  const int64_t start_time = esp_timer_get_time(); // us

  server.handleClient();

  switch (STATUS)
  {
    case STATUS_IDLE: logicIdle(); break;
    case STATUS_RUNNING: logicRunning(); break;
    default: break;
  }

  const int64_t time_taken = esp_timer_get_time() - start_time; // us

  //Serial.printf("loop() levou %"PRId64"ms de 125.", time_taken / 1000);
  //Serial.println();

  // o delay é calculado pro loop executar aproximadamente 8 vezes por segundo (ou seja, intervalos de 125ms)
  delay(std::max(125 - (time_taken / 1000), (int64_t)0));
}