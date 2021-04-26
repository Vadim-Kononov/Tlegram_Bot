/*
Notepad++ регулярное выражение
удаление части строки от начнала до \
^.*?\\

Установка разрешений для файла
sudo chmod +x /home/pi/bot/bot
sudo chmod +x /home/pi/bot/restart.sh

Узнать ID_процесса
ps aux | grep bot

Прервать процесс по ID
kill ID_процесса

Прервать процесс по имени
pkill bot

Запуск в фоне
/home/pi/bot/bot &

Засыпающий Wlan
sudo -i
sudo apt-get update && apt-get upgrade
sudo iw wlan0 get power_save
sudo iw wlan0 set power_save off
Добавить строку в /etc/rc.local
/sbin/iwconfig wlan0 power off

dmesg | grep watchdog

Строка для компиляции
g++ bot.cpp -o bot -lcurl -Wno-psabi
*/
#include <vector>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <cstring>					//для strcpy
#include <sstream>					//для stringstream
#include <random>					//для mt19937 mersenne
#include <curl/curl.h>
#include <ctime>
#include "nlohmann/json.hpp"

#define SIZE_SPEECH     1000													//Длина фразы
#define SIZE_OFILE     	500 * SIZE_SPEECH										//Количество фраз в файле
#define ADDRESS_COUNTER SIZE_SPEECH / 2											//Смещение счетчика фраз
#define ADDRESS_CURSOR  ADDRESS_COUNTER + 100									//Смещение номера последней выбранной фразы

using namespace std;
using json = nlohmann::json;

//string str_token = "1594675437:AAF_Il5Or9wOMWTUaJq4Fxt1pjcMIql2Zdw";			//@test737373bot
string str_token = "1674297897:AAFfezPbJvLHUw8a7s9D3q0zrtGO9ZskGRk";			//@HulyaSuperBot

int64_t
chat_id,						//Номер чата обновления API Telegram bot
timer;							//Отсчет повторения цикла

const int64_t
test_id = -593775528,			//@test737373bot
gus_id 	= -597386992;			//Номер основного чата

int32_t
update_id,						//Номер обновления 		API Telegram bot
message_id,						//Номер сообщения 		API Telegram bot
user_id;						//Номер юзера 			API Telegram bot

const int32_t
main_id 	= 1067229173,      //Номера юзеров
gordei_id	= 1531122286,
plot_id 	= 388900696,
valera_id	= 1640573929,
olegon_id 	= 1571892320,
tatyana_id	= 1692895631,
dimon_id 	= 96728541,
vantey_id	= 0,
noname_id	= 0;

int16_t
minute_current,					//Текущая минута
minute_previous,				//Предшествующая минута
hour_current,					//Текущий час
silence_counter,				//Счетчик пропущенных постов в режиме #заткнись
rand_min,						//Минимальное случайное число
rand_max,						//Максимальное случайное число
post_density,					//Плотность посов шт./мин.
post_counter;					//Счетчик постов


string
str_first_name,					//Первое имя юзера 				API Telegram bot
str_last_name,					//Второе имя юзера				API Telegram bot
str_title,						//Имя чата						API Telegram bot
str_text,						//Текст полученного сообщения 	API Telegram bot
str_caption,					//Комментарий к медиа файлу 	API Telegram bot
curlBuffer,						//Буфер для ответа на POST запрос
req_name,						//Наименование POST запроса
arg_1,							//Первый аргумет POST запроса
arg_2;							//Второй аргумент POST запроса

vector <string> str_file_id(6); //Массив для передачи id файлов от медиа постов API Telegram bot

vector <int32_t> 			name_id = 	{																//Массив с номерами юзеров
										main_id,
										gordei_id,
										plot_id,
										valera_id,
										olegon_id,
										tatyana_id,
										dimon_id,
										vantey_id,
										noname_id
										};
										
vector < vector <int> > 	name_var = 	{																//Массив со счетчиками постов юзеров и счетчиками минут юзеров
										{0, 0},
										{0, 0},
										{0, 0},
										{0, 0},
										{0, 0},
										{0, 0},
										{0, 0},
										{0, 0},										
										{0, 0},
										};
										
vector < vector <string> > 	name_nik = 	{																//Массив с никами юзеров
										{"Кон", 		"Кончик", 		"Папа", 			"Конище"	},
										{"Гордей", 		"Гарди", 		"Сантер", 			"Александер"},
										{"Плот", 		"Плотик", 		"Сынку", 			"Вадюха"	},
										{"Крапивин", 	"Валерьян", 	"Кропаль", 			"Валера-Сан"},
										{"Олежон", 		"Олега", 		"Олег Иванович", 	"Олежечка"	},
										{"Таня", 		"Танюха", 		"Танечька", 		"Татьяна"	},
										{"", 			"", 			"", 				""			},
										{"Вантей", 		"Дрюня", 		"Макаревич", 		"Андрюха"	},
										{"", 			"", 			"", 				""			},
										};
										
bool
flag_ok,				//true, если в ответе на POST пришло "ok"
flag_video,				//true, если пришло видео
flag_audio,				//true, если пришло аудио
flag_voice,				//true, если пришло голосовое сообщение
flag_photo,				//true, если пришла картинка
flag_sticker,			//true, если пришел стикер
flag_document,			//true, если пришел документ
flag_location,			//true, если пришло местоположение
flag_silence;			//true, если режим #заткнись

#include "functions.cpp"	//Подключение всех функций из отдельного файла


int main()
{
cout << "\n\n\n++++++++++++++++++++++++++ Запуск ++++++++++++++++++++++++++\n\n\n";
timer		= clock();												//Сохранение тиков
PutInt ("post_counter", 0);											//Обнуление счетчиков постов для режима #заткнись
silence_counter = GetInt("silence_setup");							//Получение количества постов с начала включения режима #заткнись
flag_silence = false;												//Режим #заткнись отключается после перезагрузки

while(true)															//Бесконечный цикл
{
	if (clock() - timer >= 1000000)									//Если прошла одна секнда
	{
		system("sudo sh -c \"echo '.' >> /dev/watchdog\"");			//Запуск-сброс сторожевого таймера
		
		timer = clock();											//Сброс счетчика тиков
		char str_time [100];										//Массив для сохранения строки с временем
		time_t now = time(0);										//Получение текущего времени
		struct tm * timeinfo; 										//Структура для извлечения временых значений
		timeinfo = localtime (&now);								//Получение местного времени
		strftime (str_time, 100, "%d.%b%t%H:%M:%S", timeinfo);		//Преобразование времени в строку
		cout << str_time << "\n";									//Вывод отметки времени в log.log
		strftime (str_time, 100, "%M", timeinfo);					//Получение текущей минуты
		minute_current = atoi(str_time); 							//Преобразование текущей минуты в целое число
		strftime (str_time, 100, "%H", timeinfo);					//Получение текущего часа
		hour_current = atoi(str_time); 								//Преобразование текущего часа в целое число
		
		if (minute_current != minute_previous)						//Если произошло изменение минуты
		{
			minute_previous = minute_current;						//Считаем эту миуту прошедшей
			PutInt ("minute_counter", GetInt("minute_counter") + 1);//Сохранене счетчика минут
			post_density = post_counter;							//Получение количества постов пришедшиш за одну минуту
			post_counter = 0;										//Обнуления счетчика постов по которому вычисляется плотнось
			
			for (int i = 0; i <= 8 ; i++) 
			{
				if (name_var[i][1] < 30) 	name_var[i][1] ++;		//Увеличение счетчика минут пользователя
				else {name_var[i][0] = 0; 	name_var[i][1] = 0;}	//Если прошло 30 минут, то сброс счетчика постов и счетчика минут
			}
		}		
		
		if (GetInt("minute_counter") >= GetInt("minute_setup"))							//Если прошло больше минут чем задано для режима ожидания
		{
			PutInt ("minute_counter", 0);												//Сброс счетчика минут для режима ожидания
			string str = SearchSpeech(2);												//Получения строки из файла с постами ожидания
			req_name = "sendMessage";													//Присвоение имени POST запроса
			Request(req_name, gus_id, str, arg_2);										//Отправка POST запроса - отправка сообщения в основной чат
		}
		
		
		//Отправка POST запроса на получение последних десяти сообщений
		req_name = "getUpdates"; arg_1 = to_string(GetInt ("update_id")); arg_2 = "10";
		if (Request(req_name, gus_id, arg_1, arg_2))  									//Если запрос успешен
		{
			cout << "\n" << curlBuffer << "\n";											//Вывод ответа от сервера
			if (Json ())																//Если распарсинг успешен
			{
				PutInt ("update_id", update_id + 1);									//Увеличение номера обновления, следующего к получению
				PutInt ("minute_counter", 0);											//Сброс счетчика минут для режима ожидания
				req_name = "sendMessage"; 												//Подготовка имени следующего POST запроса
			
				int name_index = 8;														//Внутренний индекс пользователя, по умолчанию равен noname
				for (int i = 0; i <= 7 ; i++) if (user_id == name_id[i]) name_index = i;//Определение номера известного пользователя по его user_id
				
				if (name_index !=8 )													//Если пользователь из числа известных
				{int i  = RandomNumber(0, 3); str_first_name = name_nik[name_index][i];}//Замена его имени на случайный ник
			
				name_var[name_index][0]++;												//Увеличение счетчика постов пользователя
				post_counter ++;														//Увеличение общего счетчика постов
				
				/**/
				
				string str_test = "";
				for (int i = 0; i <= 8 ; i++)
				{
				str_test += "name_var[" + to_string(i) + "][0]" + ": " + to_string(name_var[i][0]) + "    " + "name_var[" + to_string(i) + "][1]"   + ": " + to_string(name_var[i][1]) + "\n";
				} 
				Request(req_name, main_id, str_test, arg_2);
				
				
				/**/
				
				//Пересылка медиа сообщений в чат, имя запроса определяется по типу медиа контента
				if 		(flag_photo && 		chat_id > 0) 	{req_name = "sendPhoto"; 		arg_1 = str_file_id.at(0); arg_2 = str_caption; Request(req_name, gus_id, arg_1, arg_2);}
				else if (flag_audio && 		chat_id > 0) 	{req_name = "sendAudio"; 		arg_1 = str_file_id.at(1); arg_2 = str_caption; Request(req_name, gus_id, arg_1, arg_2);}
				else if (flag_video && 		chat_id > 0) 	{req_name = "sendVideo"; 		arg_1 = str_file_id.at(2); arg_2 = str_caption; Request(req_name, gus_id, arg_1, arg_2);}
				else if (flag_voice && 		chat_id > 0) 	{req_name = "sendVoice"; 		arg_1 = str_file_id.at(3); arg_2 = str_caption; Request(req_name, gus_id, arg_1, arg_2);}
				else if (flag_document && 	chat_id > 0) 	{req_name = "sendDocument"; 	arg_1 = str_file_id.at(4); arg_2 = str_caption; Request(req_name, gus_id, arg_1, arg_2);}
				else if (flag_sticker && 	chat_id > 0) 	{req_name = "sendAnimation"; 	arg_1 = str_file_id.at(5); arg_2 = str_caption; Request(req_name, gus_id, arg_1, arg_2);}
				
				//Пересылка текста в основной чат от имени бота, если он располагается за двумя слэшами в начале строки
				else if (str_text.find("//") == 0 && chat_id > 0) {str_text.erase(0, 2); Request(req_name, gus_id, str_text, arg_2);}	
				
				//Обработка строки, как команды, если слэш в числе первых четырех символов, вызов CommandLine ()
				else if ((str_text.find("/") == 0 || str_text.find("/") == 1 || str_text.find("/") == 2 || str_text.find("/") == 3) && chat_id > 0)
				{arg_1 = CommandLine (); Request(req_name, chat_id, arg_1, arg_2);}
 				
				//Ответы в чат и в приват в режимах: ответ по ключу, случайный ответ						
				else if (!flag_silence)																//Если режим #заткнись не активен
				{
					string str = LowCase(str_text);													//Перевод принятой строки в нижний регистр (только кириллица)
					int i = SearchKey(str);															//Поиск файла соответсвующего ключу из Keywords.txt
					if (i == 3) {flag_silence = true; silence_counter = GetInt("silence_setup");}	//Если это третий файл, устанавливается режим #заткнись и счетчик постов
					str = SearchSpeech(i);															//Поиск случайной строки ответа, в соответсвующем файле
					//Ответ по ключу
					if (str.length() != 0) SendingPost (str);										//Если возвращена не пустая строка, то отправка ответа
					else																			//Если ответ не найден то отправка случайной строки из файла 1.txt
					{
						if (post_density < 3) 	{rand_min = 2; rand_max = 4;}						//Если за предшествующую минуту всего было меньше трех постов, то ответ на каждый 4-5 пост конкретного пользователя
						else 					{rand_min = 5; rand_max = 7;}						//Если за предшествующую минуту всего было не меньше трех постов, то ответ на каждый 7-8 пост конкретного пользователя
						
						if (name_var[name_index][0] > RandomNumber(rand_min, rand_max))				//Проверка количества постов, поступивших от пользователя
						{
							name_var[name_index][0] = 0;											//Сброс счетчика постов пользователя
							name_var[name_index][1] = 0;											//Сброс счетчика минут пользователя
							str = SearchSpeech(1);													//Получение случайного ответа из файла 1.txt
							SendingPost (str);														//Отправка случайного ответа
						}
					}
				}
				else if (flag_silence)																//Если режим #заткнись активен
				{
					silence_counter --;																//Уменьшения счетчика прошедших в режиме #заткнись постов
					if (silence_counter <=0) flag_silence = false;									//Если счетчик уменьшился до нуля, отключение режима #заткнись				
				}
				cout << "\n" << curlBuffer << "\n";													//Вывод ответа от сервера на запрос send...
			}
		}
	}
}
return 0;
}