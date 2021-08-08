/*
Функция, вызываемая cURL для записи полученых данных
*/
size_t curlWriteFunc(char *data, size_t size, size_t nmemb, std::string *buffer)  
{  
	size_t result = 0;  
	if (buffer != NULL) {buffer->append(data, size * nmemb); result = size * nmemb;}
    return result;  
}  



/*
POST запрос выполняемый cURL, строка с именем запроса, номер чата, три строковых аргумента
*/
int Request(string &req_name, int64_t chat_id, string &arg_1, string &arg_2)
{
  char curlErrorBuffer[CURL_ERROR_SIZE];											//Буфер для сохранения кода ошибки от curl
  string str_URL = "https://api.telegram.org/bot" + str_token + "/" + req_name;	//Заголовок POST запроса
  string str_parameter;															//Строка с параметрами POST запроса
  
  if 		(req_name == "getUpdates")		str_parameter = "offset="  + arg_1 + "&limit=" +  arg_2 + "&timeout=2";
  else if 	(req_name == "sendMessage") 	str_parameter = "chat_id=" + to_string(chat_id) + "&text=" 		+ arg_1;
  else if 	(req_name == "sendPhoto")		str_parameter = "chat_id=" + to_string(chat_id) + "&photo=" 	+ arg_1 + "&caption=" + arg_2;
  else if 	(req_name == "sendVideo")		str_parameter = "chat_id=" + to_string(chat_id) + "&video=" 	+ arg_1 + "&caption=" + arg_2;
  else if 	(req_name == "sendAnimation")	str_parameter = "chat_id=" + to_string(chat_id) + "&animation=" + arg_1 + "&caption=" + arg_2;
  else if 	(req_name == "sendAudio")		str_parameter = "chat_id=" + to_string(chat_id) + "&audio=" 	+ arg_1 + "&caption=" + arg_2;
  else if 	(req_name == "sendVoice")		str_parameter = "chat_id=" + to_string(chat_id) + "&voice=" 	+ arg_1 + "&caption=" + arg_2;
  else if 	(req_name == "sendDocument")	str_parameter = "chat_id=" + to_string(chat_id) + "&document=" 	+ arg_1 + "&caption=" + arg_2; 
  
  
  CURL *curl = curl_easy_init();										//Инциализация cURL в режиме easy
  if(curl) 
  {
	
    curl_easy_setopt(curl, CURLOPT_URL, str_URL.c_str());				//URL
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, str_parameter.c_str());//Параметры POST
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 4000);					//Таймаут ожидания ответа сервера
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlBuffer);			//Параметр сохранения ответа сервера
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunc);		//Функция вызываемая cURL для записи полученых данных
	curlBuffer = "";														//Очистка буфера ответа
    CURLcode curlResult = curl_easy_perform(curl);						//Выполнение запроса
	
    if (curlResult == CURLE_OK)  {} 										//Успешный запрос
	
	else																	//Сервер вернул ошибку
	{
	cout << "Ошибка(" << curlResult << "): " << curlErrorBuffer << endl;
	curl_easy_cleanup(curl);
	system("sudo sh -c \"echo '.' >> /dev/watchdog\"");					//Толчок для WatchDog перед выходом					
	return(0);
	}
    
	curl_easy_cleanup(curl);												//Завершение сеанса
  	system("sudo sh -c \"echo '.' >> /dev/watchdog\"");					//Толчок для WatchDog перед выходом
	return 1;
  }
  curl_global_cleanup();
  system("sudo sh -c \"echo '.' >> /dev/watchdog\"");						//Толчок для WatchDog перед выходом	
  return 0;	
}



/*
Парсинг ответа сервера
*/
int Json ()
{
    json message, from, chat;
	json answer 	= json::parse(curlBuffer);
	flag_ok	= answer["ok"];
	json result 	= answer["result"][0];
	if (result.size())
	{	
		update_id 	= result["update_id"];  
		
		if 		(result["message"].size()) 				
		{
			message 	= result["message"];	
			if(message.size()) 	message_id 	= message["message_id"]; else message_id = 0;
		}
		
		else if (result["edited_message"].size()) 
		{
			message 	= result["edited_message"];	
			if(message.size()) 	message_id 	= message["message_id"]; else message_id = 0;
		}
		
		else if (result["my_chat_member"].size()) 		message = result["my_chat_member"];
		
		if (message["from"].size()) 					from 	= message["from"];
		if (message["chat"].size()) 					chat 	= message["chat"];
		
		if(from.size())		user_id 	= from["id"]; else user_id = 0;
		if(chat.size())		chat_id 	= chat["id"]; else chat_id = 0;
		
		if (message["photo"].size()) flag_photo 		= true; else flag_photo 	= false;
		if (message["audio"].size()) flag_audio 		= true; else flag_audio 	= false;
		if (message["video"].size()) flag_video 		= true; else flag_video 	= false;
		if (message["voice"].size()) flag_voice 		= true; else flag_voice 	= false;
		if (message["document"].size()) flag_document 	= true; else flag_document 	= false;
		if (message["sticker"].size()) flag_sticker 	= true; else flag_sticker 	= false;
		if (message["location"].size()) flag_location 	= true; else flag_location 	= false;
	
		if (message["photo"][0]["file_id"].is_string()) str_file_id.at(0) 	= message["photo"][0]["file_id"]; 	else str_file_id.at(0) 	= "";
		if (message["audio"]["file_id"].is_string()) 	str_file_id.at(1) 	= message["audio"]["file_id"];		else str_file_id.at(1) 	= "";	
		if (message["video"]["file_id"].is_string()) 	str_file_id.at(2) 	= message["video"]["file_id"];		else str_file_id.at(2) 	= "";		
		if (message["voice"]["file_id"].is_string()) 	str_file_id.at(3) 	= message["voice"]["file_id"];		else str_file_id.at(3) 	= "";	
		if (message["document"]["file_id"].is_string()) str_file_id.at(4) 	= message["document"]["file_id"];	else str_file_id.at(4) 	= "";
		if (message["sticker"]["file_id"].is_string()) 	str_file_id.at(5) 	= message["sticker"]["file_id"];	else str_file_id.at(5) 	= "";	
		if (message["caption"].is_string()) 			str_caption 		= message["caption"]; 				else str_caption 		= "";
		if (from["first_name"].is_string()) 			str_first_name 		= from["first_name"];				else str_first_name 	= "";
		if (from["last_name"].is_string()) 				str_last_name 		= from["last_name"];				else str_last_name 		= "";
		if (chat["title"].is_string()) 					str_title 			= chat["title"];					else str_title 			= "";
		if (message["text"].is_string()) 				str_text 			= message["text"]; 					else str_text 			= "";
	return 1;
	}
	else return 0;
}



/*Генерация случайного числа в указанном диапазоне с использованием алгоритма Вихря Мерсенна,
min и max включаются в генерируемый диапазон
*/
int RandomNumber(int min, int max)
{
random_device mer; 											//Определение для генератора случайных чисел
mt19937 mersenne(mer());										//Генерация случайного числа
return(mersenne() % (max - min + 1) + min);					//Приведение случайного числа в нужный диапазон
}



/*
Функция перевола первого символа строки в верхний регистр,
только для кириллицы
*/
string FirstCharUp (string &stroka)
{
char * c = new char [stroka.length()+1];
strcpy (c, stroka.c_str());
  if        (c[0] == 208 && c[1] >=176 && c[1] <=191) c[1] -= 32;
  else if   (c[0] == 209 && c[1] >=128 && c[1] <=143) {c[0] = 208; c[1] += 32;}
  else if   (c[0] == 209 && c[1] == 145 ) {c[0] = 208; c[1] = 129;}
return ((string)c);
}



/*
Функция перевода всех символов строки в нижний регистр,
только для кириллицы
*/
string LowCase (string &stroka)
{
char * c = new char [stroka.length()+1];
strcpy (c, stroka.c_str());
for (int i = 0; i<= stroka.length(); i +=1)
  {
  if        (c[i] == 208 && c[i + 1] >=144 && c[i + 1] <=159) {c[i + 1] = c[i + 1] + 32;}
  else if   (c[i] == 208 && c[i + 1] >=160 && c[i + 1] <=175) {c[i] = 209; c[i + 1] = c[i + 1] - 32;}
  else if   (c[i] == 208 && c[i + 1] == 129 ) {c[i] = 209; c[i + 1] = 145;}
  }  
return ((string)c);
}



/*
Сохранение знаковой целой переменной int64_t в файле,
имя переменной строка, значение
*/
int PutInt (const string name, int64_t value)
{
string file_name = "/home/kononov/bot/var/" + name + ".txt";	
ofstream file(file_name);
string str = to_string(value);								//Преобразование int -> string
file << str << "\n";
file.close();
return 1;	
}



/*
Извлечение знаковой целой переменной int64_t хранящейся в файле
имя переменной строка
*/
int64_t GetInt (const string& name)
{
string file_name = "/home/kononov/bot/var/" + name + ".txt";	
ifstream file(file_name);
if (!file) return 0;   										//При отсутсвии файла возвращается 0
string str;
getline(file, str, '\n');
stringstream stream(str);
int64_t result; stream >> result;							//Преобразование string -> int
file.close();
return result;													//Возвращает int64_t
}



/*
Проверка наличия файлов типа 1.txt в рабочей папке, возвращает номер последнего найденного файла,
проверка последовательная от 1 до 999
*/
int NDir ()
{
for (int i = 1; i<= 999; i++)	
{
	string file_name = "/home/kononov/bot/data/" + to_string(i) + ".txt";	//Имя файла с распределенными по смещениям фразами 1.txt
	ifstream file(file_name);															
	if (!file) return i - 1;  										//Проверка наличия файла
	file.close();
}
return 1000;															//Возврат 1000 в случае ошибки
}



/*
Проверка наличия файлов типа i1.txt в рабочей папке, возвращает номер последнего найденного файла,
проверка последовательная от 1 до 999
*/
int NiDir ()
{
for (int i = 1; i<= 999; i++)	
{
	string file_name = "/home/kononov/bot/data/i" + to_string(i) + ".txt";	//Имя файла с линейными фразами i1.txt
	ifstream file(file_name);
	if (!file) return i - 1;  											//Проверка наличия файла
	file.close();
}
return 1000;																//Возврат 1000 в случае ошибки
}



/*
Проверка наличия файлов типа 1.txt в рабочей папке, возвращает имя последнего найденного файла,
проверка последовательная от 1 до 999
*/
string LDir ()
{
string str = "";
for (int i = 1; i<= 999; i++)	
{
	string file_name = "/home/kononov/bot/data/" + to_string(i) + ".txt";	//Имя выходного файла с распределенными по смещениям фразами 1.txt
	ifstream file(file_name);
	if (!file) return str + "\nПоиск завершен";						//Проверка наличия файла
	else str += "Найден файл " + to_string(i) + ".txt\n";
	file.close();
}
return "";																//Возвращает пустую строку в случае ошибки
}



/*
Чтение из файла с распределенными строками
Смещение 	0: 						ключевая фраза
Смещение	SIZE_SPEECH/2: 			счетчик фраз в файле
Смещение	SIZE_SPEECH/2 + 100:	номер последней выбранной фразы
*/
string FileRead (int file_number)
{
	string file_name = "/home/kononov/bot/data/" + to_string(file_number) + ".txt";
	fstream file(file_name, ios::in);													//Открытие файла одновременно на чтение
	if (!file) return "Файл " + to_string(file_number) + ".txt не найден\n";			//Проверка наличия файла
	string str = "", str_out = "";														//Промежуточная строка, выходная строка
	file.seekg(ADDRESS_COUNTER); getline(file, str, '\n'); int j = atoi(str.c_str());//Получение количества строк
	file.seekg(ADDRESS_CURSOR); getline(file, str, '\n'); int k = atoi(str.c_str());	//Получение номера последней выбранной фразы
	str_out = "Файл " + to_string(file_number) + ".txt\n_____________\n";				//Строка заголовка
	str_out += "n: " + to_string(j) + " | s: " + to_string(k) + "\n";					//Строка со значениями количества строк и номером последней выбранной фразы
	
	for (int i = 0; i<= j; i++)
	{
	file.seekg(i*SIZE_SPEECH);															//Установка указателя
	getline(file, str, '\n');																//Чтение строки до '\n'
	if (str_out.length() + str.length() > 4000)											//Проверка длины формируемой строки 
	{Request(req_name, chat_id, str_out, arg_2); str_out = "";}						//Если строка длинее 4000 символов, отправка этой строки и обуление
	if 		(i == 0) str_out += "key: "	+ str + "\n\n";									//Строка с ключом
	else if	(i != 0) str_out +=	to_string(i) + ". " + str + "\n";						//Строки с фразами
	}
	file.close();																			//Закрытие файла
	Request(req_name, chat_id, str_out, arg_2);											//Отправка набранной строки
	return "";																				//Возврат пустой строки, т.к. все уже отправлено
}



/*
Чтение из линейного файла
*/
string FileReadLinear (const string name)
{
	string file_name = "/home/kononov/bot/data/" + name;
	fstream file(file_name, ios::in);													//Открытие файла на чтение
	if (!file) return "Файл " + name + "не найден\n";										//Проверка наличия файла
	int i = 0; string str = "", pre_str = "", out_str = "";								//Счетчик строк, промежуточная строка, префиксная строка, выходная строка
	do																				
	{																						//Пока не достигнут конец файла
	getline(file, str, '\n');																//Чтение строки до '\n'
	i++; if (name == "Keywords.txt") pre_str = to_string(i) + ". ";						//Для файла Keywords.txt вывод отдельного префикса
	out_str +=  pre_str + str + "\n";														//Формирование строк из префикса и содержимого
	}
	while (!file.eof());																	//Проверка достижения конца входного файла
	
	file.close();																			//Закрытие файла
	return out_str;																		//Возврат строки для сообщения
}



/*
Запись из файлов с поледовательно идущими строками
в файлы со строками разнесенными по смещениям для возможности замены.
Смещение 	0: 						ключевая фраза
Смещение	ADDRESS_COUNTER: 		счетчик фраз в файле
Смещение	ADDRESS_CURSOR:			номер последней выбранной фразы
*/
string FileFormat (int start_number, int end_number)
{
system("sudo sh -c \"echo 'V' >> /dev/watchdog\"");										//Остановка WatchDog в связи с длительностью операции
string str_out = "";																		//Определение выходной строки
for (int i = start_number; i<= end_number; i++)	
{
	string ifile_name = "/home/kononov/bot/data/i" + to_string(i) + ".txt";					//Имя входного файла i1.txt
	string ofile_name = "/home/kononov/bot/data/" + to_string(i) + ".txt";					//Имя выходного файла с распределенными по смещениям фразами 1.txt
	
	ifstream ifile(ifile_name);															//Открытие входного файла
	if (!ifile) {str_out += "i" + to_string(i) + ".txt не найден\n"; return str_out;}   //Проверка наличия входного файла
	
	fstream ofile(ofile_name, ios::out | ios::trunc);									//Открытие выходного файла с распределенными по смещениям фразами
	for (int i = 1; i <= SIZE_OFILE; i++) ofile.put ('\0');								//Полное заполнение выходного файла символами NULL
	
	int j = 0; uint32_t point = 0; string str = "";										//Счетчик фраз, указатель смещения, промежуточная строка
	ifile.seekg(0);
	getline(ifile, str, '\n');															//Чтение первой строки из входного файла
	ofile.seekp(point);																	//Указатель выходного файла в ноль
	ofile << str << "\n";																	//Запись первой, ключевой строки
	
	do																						//Пока не достигнут конец линейного файла
	{
	j++;																					//Инкремент счетчика фраз
	getline(ifile, str, '\n');															//Чтение строки из входного файла до '\n'
	point += SIZE_SPEECH;																	//Вычисление смешения в выходном файле
	ofile.seekp(point);																	//Установка указателя
	ofile << str << "\n";																	//Запись строки в выходной файл
	}
	while (!ifile.eof());																	//Проверка достижения конца входного файла
	
	ofile.seekp(ADDRESS_COUNTER); ofile << j << "\n";									//Запись числа фраз
	ofile.seekp(ADDRESS_CURSOR);  ofile << 1 << "\n";									//Сброс номера последней выбранной фразы
	str_out += "i" + to_string(i) + ".txt -> " + to_string(i) + ".txt\n";				//Формирование выходной строки
	ifile.close(); ofile.close();														//Закрытие входного и выходного файлов
}
return str_out;																			//Возврат сформированной строки
}



/*
Запись из файлов с разнесенными по смещениям строками
в файлы с поледовательно идущими строками
Смещение 	0: 						ключевая фраза
Смещение	ADDRESS_COUNTER: 		счетчик фраз в файле
Смещение	ADDRESS_CURSOR:			номер последней выбранной фразы
*/
string ReverseFormat (int start_number, int end_number)
{
system("sudo sh -c \"echo 'V' >> /dev/watchdog\"");										//Остановка WatchDog в связи с длительностью операции
string str_out = "";																		//Определение выходной строки
for (int i = start_number; i<= end_number; i++)	
{
	string ifile_name = "/home/kononov/bot/data/" + to_string(i) + ".txt";					//Имя входного файла с распределенными по смещениям фразами 1.txt
	string ofile_name = "/home/kononov/bot/data/i" + to_string(i) + ".txt";					//Имя выходного файла с линейными строками i1.txt
	
	ifstream ifile(ifile_name);															//Открытие входного файла
	if (!ifile) {str_out += "Файл " + to_string(i) + ".txt не найден\n"; return str_out;}//Проверка наличия входного файла
	
	fstream ofile(ofile_name, ios::out | ios::trunc);									//Открытие выходного файла с распределенными по смещениям фразами
	string str = "";																		//Промежуточная строка
	ifile.seekg(ADDRESS_COUNTER); getline(ifile, str, '\n'); int j = atoi(str.c_str());	//Получение количества строк
	
	for (int i = 0; i<= j; i++)
	{
	ifile.seekg(i*SIZE_SPEECH);															//Установка указателя
	getline(ifile, str, '\n');															//Чтение строки до '\n'
	ofile << str; if (i !=j ) ofile << "\n";												//Запись строки в выходной файл, если строка не последняя то добавление "\n"
	}
	str_out += to_string(i) + ".txt -> i" + to_string(i) + ".txt\n";						//Формирование выходной строки
	ifile.close(); ofile.close();														//Закрытие входного и выходного файлов
}
return str_out;																			//Успешное завершение, возврат строки
}



/*
Запись в файл Keywords.txt заголовков-ключей из отформатированных файлов
*/
string KeyFormat (int start_number, int end_number)
{
system("sudo sh -c \"echo 'V' >> /dev/watchdog\"");											//Остановка WatchDog в связи с длительностью операции
string str_out = "";																			//Определение выходной строки
		
string ofile_name = "/home/kononov/bot/data/Keywords.txt";
ofstream ofile(ofile_name);																	//Открытие Keywords.txt на запись
int i;																							//Счетчик файлов
for (i = start_number; i<= end_number; i++)													//От первого до последнего файлов
{	
	string ifile_name = "/home/kononov/bot/data/" + to_string(i) + ".txt", str;					//Имя форматированного файла 1.txt
	ifstream ifile(ifile_name);																//Открытие форматированного файла на чтение										
	if (!ifile) {str_out += "Файл " + to_string(i) + ".txt не найден\n"; return str_out;}  //Проверка наличия файла
	ifile.seekg(0);																			//Установка указателя
	getline(ifile, str, '\n');																//Получение строки с ключом
	if (i != end_number) ofile << str + "\n"; else ofile << str;							//Исключение символа перевода каретки для последней записываемой строки и запись в Keywords.txt
	str_out +=  to_string(i) + ".txt -> Key\n";												//Формирование выходной строки
	ifile.close();  					
}
ofile.close();
return str_out;																				//Успешное завершение, возврат строки
}



/*
Создание нового файла с разнесенными строками и записью ключевой фразы
Смещение 	0: 						ключевая фраза
Смещение	ADDRESS_COUNTER: 		счетчик фраз в файле
Смещение	ADDRESS_CURSOR:			номер последней выбранной фразы
*/
string NewFile (int number, string& str)
{
string str_out = "";
string file_name = "/home/kononov/bot/data/" + to_string(number) + ".txt";				//Имя выходного файла с распределенными по смещениям фразами 1.txt
ofstream file(file_name);																//Открытие выходного файла с распределенными по смещениям фразами
for (int i = 1; i <= SIZE_OFILE; i++) file.put ('\0');								//Полное заполнение выходного файла символами NULL
file.seekp(0);																			//Указатель выходного файла в ноль
file << str << "\n";																	//Запись первой, ключевой строки
file.seekp(ADDRESS_COUNTER); file << 0 << "\n";										//Запись количества строк
file.seekp(ADDRESS_CURSOR);  file << 1 << "\n";										//Установка номера последней выбранной фразы
file.seekp(SIZE_SPEECH);  	 file << "\n";											//Запись первой, пустой фразы
str_out = "Файл " + to_string(number) + ".txt создан\n";								//Формирование выходной строки
file.close();																			//Закрытие файла
KeyFormat(1, NDir ());																//Вызов функции записи в файл Keywords.txt
FileRead (number);																		//Вызов функции чтения и отправки содержимого файла
return str_out;																		//Успешное завершение, возврат строки
}



/*
Поиск номера файла для ответа по ключевому слову.
Ищет во входной строке подстроку из файла Keywords.txt и
возвращает номер файла для открытия типа 1.txt
*/
int SearchKey (string& incoming_string)
{
	ifstream file("/home/kononov/bot/data/Keywords.txt");										//Открытие файла Keywords.txt для чтения
	if (!file) {cerr << "Ошибка открытия data/Keywords.txt" << "\n"; return -1;}			//Проверка наличия файла
	int i=0; string str;																	//Счетчик строк, промежуточная строка
	while (file)																			//Пока не достигнут конец файла
	{
	i++;																					//Счетчик строк
	getline(file, str, '\n');																//Извлечение строки из файла до \n
	if (strstr( incoming_string.c_str(), str.c_str()) != nullptr)  {file.close(); return i;}//Поиск подстроки, соответсвующей ключу
	}
	file.close();																			//Закрытие файла
	return 0;																				//Если подстрока не нашлась, то выход с возвратом нуля
}



/*
Поиск фразы для случайного ответа в файле отформатированным FileFormat()
c разнесенными по смещениям строками
*/
string SearchSpeech (int file_number)
{
	string file_name = "/home/kononov/bot/data/" + to_string(file_number) + ".txt";			//Имя входного файла типа 1.txt
	fstream file(file_name, ios::in | ios::out);										//Открытие входного файла одновременно на чтение и запись
	if (!file) {cerr << "Ошибка открытия " << file_name << "\n"; return "";}			//Проверка наличия входного файла
	
	int i = 0, j = 0; uint32_t point = 0; string str = "";								//Счетчик фраз, номер последней прочитанной фразы, указатель смещения, промежуточная строка
	
	file.seekg(ADDRESS_COUNTER); getline(file, str, '\n'); i = stoi(str);				//Чтение количества строк
	file.seekg(ADDRESS_CURSOR);  getline(file, str, '\n'); j = stoi(str);   			//Чтение номера последней прочитанной фразы
	
    if (i > 10)																			//Если файл содержит более десяти фраз, то случайная выборка
	{
		while (1) {int k = RandomNumber(1, i); if (j != k) {j = k; break;}}			//Генерация случайного числа, пока оно не станет отличным от последней прочитанной фразы 
		file.seekp(ADDRESS_CURSOR);  file << j << "\n";									//Запись в файл нового номера фразы																				//cout << j << '\n';											
		file.seekg(j * SIZE_SPEECH);														//Установка указателя на новую фразу															
		getline(file, str, '\n');															//Чтение новой фразы	
	}
	else																					//Если файл содержит не более десяти фраз, то последовательная  выборка
	{
		file.seekg(j * SIZE_SPEECH);														//Установка указателя на фразу															
		getline(file, str, '\n');															//Чтение новой фразы	
		j += 1; if (j > i) j = 1;															//Вычисление нового значения номера для следующей, читаемой фразы
		file.seekp(ADDRESS_CURSOR);  file << j << "\n";									//Запись в файл нового номера фразы																				//cout << j << '\n';											
	}
	file.close();																			//Закрытие файл
	return str;																			//Успешный выход с передачей фразы
}



//Удаление файлов типа 1.txt
string DeleteFile (int start_number, int end_number)
{
string str_out = "";
for (int i = start_number; i<= end_number; i++)	
{
	string file_name = "/home/kononov/bot/data/" + to_string(i) + ".txt";					
	if (remove(file_name.c_str()) == 0) str_out += "Файл " + to_string(i) + ".txt удален\n";
	else str_out += "Ошибка: " + to_string(i) + ".txt " + strerror(errno) + "\n";
}
return str_out;	
}



//Удаление файлов типа i1.txt
string DeleteIFile (int start_number, int end_number)
{
string str_out = "";
for (int i = start_number; i<= end_number; i++)	
{
	string file_name = "/home/kononov/bot/data/i" + to_string(i) + ".txt";					
	if (remove(file_name.c_str()) == 0) str_out += "Файл i" + to_string(i) + ".txt удален\n";
	else str_out += "Ошибка: i" + to_string(i) + ".txt " + strerror(errno) + "\n";
}
return str_out;	
}



/*
Запись в существующий файл с распределенными строками новой фразы, нового ключа, счетчиков, редактирование фраз
Смещение 	0: 						ключевая фраза
Смещение	SIZE_SPEECH/2: 			счетчик фраз в файле
Смещение	SIZE_SPEECH/2 + 100:	номер последней выбранной фразы
*/
string FileWrite (int file_number, string& str_2, string& str_3)
{
	uint32_t point = 0; string str = "", str_out = "";															//Указатель смещения, промежуточная строка, строка для вывода сообщения
	int digit_str_2, digit_str_3; bool flag_str_2, flag_str_3;													//Числовое представление аргументов, и флаги его наличия
	try {digit_str_2 = stoi(str_2); flag_str_2 = true;} catch(std::invalid_argument e) {flag_str_2 = false;} 	//Превод в число второго аргумента
	try {digit_str_3 = stoi(str_3); flag_str_3 = true;} catch(std::invalid_argument e) {flag_str_3 = false;} 	//Превод в число третьего аргумента
	
	string file_name = "/home/kononov/bot/data/" + to_string(file_number) + ".txt";	//Имя файла с распределенными по смещениям фразами типа 1.txt
	fstream file(file_name, ios::in | ios::out);								//Открытие файла одновременно на чтение и запись
	if (!file) return "Ошибка открытия " + file_name + "\n";   					//Проверка наличия файла
	
	//Запись счетчика строк в файле
	if (str_2 == "n" && digit_str_3 > 0 && digit_str_3 < 1000 && flag_str_3)
		{file.seekp(ADDRESS_COUNTER); file << to_string(atoi(str_3.c_str())) << "\n"; file.close(); str_out = "Записан счетчик строк\n________________________\n";}
	
	//Запись указателя последней прочитанной строки
	else if (str_2 == "s" && digit_str_3 > 0 && digit_str_3 < 1000 && flag_str_3)
		{file.seekp(ADDRESS_CURSOR); file << to_string(atoi(str_3.c_str())) << "\n"; file.close(); str_out = "Записан указатель строк\n__________________________\n";}
	
	//Запись ключевой фразы
	else if (str_2 == "0" && str_3.length() != 0)
		{file.seekp(0); file << str_3 << "\n"; file.close(); str_out = "Записана ключевая фраза\n__________________________\n"; KeyFormat(1, NDir ());}	
	
	//Редактирование записанной фразы
	else if (digit_str_2 > 0 && digit_str_2 < 1000 && flag_str_2)
		{point = atoi(str_2.c_str())*SIZE_SPEECH; file.seekp(point); file << str_3 << "\n"; file.close(); str_out = "Заменена фраза\n_________________\n";}
	
	//Запись новой фразы в конец файла
	else if (!flag_str_2 && str_2.length() != 0 && str_3.length() == 0 )
	{
		file.seekg(ADDRESS_COUNTER);
		getline(file, str, '\n');
		point = (atoi(str.c_str()) + 1) * SIZE_SPEECH;
		file.seekp(point); file << str_2 << "\n";
		file.seekp(ADDRESS_COUNTER);
		file << to_string(atoi(str.c_str()) + 1) << "\n";
		file.close();
		str_out = "Фраза записана в конец файла\n_______________________________\n";
	}
	
	//Чтение содержимого файла (проверка аргументов)
	else if (str_2.length() == 0 && str_3.length() == 0) ;
	
	//Сообщение о неверной команде
	else str_out =  "Неверная команда\n___________________\n";
	
	file.close();
	Request(req_name, chat_id, str_out, arg_2);									//Отправка сообщения о результотах
	FileRead (file_number);														//Отправка модифицированного файла
	return "";
}



/*
Функция обработки командной строки содержащей прямой слэш
*/
string CommandLine ()
{
	string str = str_text, str_1, str_2, str_3;
																												//Разбиение входной строки на три подстроки
	if (str.find("/") != string::npos) {str_1 = str.substr (0, str.find("/")); str.erase(0, str.find("/")+1);} else {str_1 = str; str = "";}
	if (str.find("/") != string::npos) {str_2 = str.substr (0, str.find("/")); str.erase(0, str.find("/")+1);} else {str_2 = str; str = "";}
	if (str.find("/") != string::npos) {str_3 = str.substr (0, str.find("/")); str.erase(0, str.find("/")+1);} else {str_3 = str; str = "";}

	int digit_str_1, digit_str_2, digit_str_3; bool flag_str_1, flag_str_2, flag_str_3;							//Числовое представление строк и флаги его наличия
	try {digit_str_1 = stoi(str_1); flag_str_1 = true;} catch(std::invalid_argument e) {flag_str_1 = false;} 	//Превод в число первого аргумента
	try {digit_str_2 = stoi(str_2); flag_str_2 = true;} catch(std::invalid_argument e) {flag_str_2 = false;} 	//Превод в число второго аргумента
	try {digit_str_3 = stoi(str_3); flag_str_3 = true;} catch(std::invalid_argument e) {flag_str_3 = false;} 	//Превод в число третьего аргумента	
		
		//Если первая подстрока не число
		//Запись строк из линейных файлов в рабочие файлы с разнесенными строками
		if (str_1 == "ff") return FileFormat(1, NiDir ()) + "\n" + KeyFormat(1, NDir ()); 
		
		//Запись ключей из первых строк файлов в файл Keywords.txt
		if (str_1 == "kf") return KeyFormat(1, NDir ()); 
		
		//Обратная запись из файлов с разнесенными строками в файлы с линейными строками
		if (str_1 == "rf") return ReverseFormat (1, NDir ());
		
		//Создание нового файла с разнесенными строками и запись в него ключевого слова
		if (str_1 == "nf" && str_2.length() != 0) return NewFile (NDir () + 1, str_2);
		
		//Удаление файлов типа 1.txt
		if (str_1 == "df" && flag_str_2 && flag_str_3) return DeleteFile (digit_str_2, digit_str_3);
		
		//Удаление файлов типа i1.txt
		if (str_1 == "di" && flag_str_2 && flag_str_3) return DeleteIFile (digit_str_2, digit_str_3);
		
		//Вывод перечня существующих файлов типа 1.txt
		if (str_1 == "l") return LDir();
		
		//Чтение содержимого файла ключей Keywords.txt
		if (str_1 == "k") return FileReadLinear ("Keywords.txt");
		
		//Запись количества минут времени ожидания активности в чате
		if (str_1 == "t" && flag_str_2) {PutInt ("minute_setup", digit_str_2); string str = "Время ожидания постов: " + to_string(digit_str_2) + " минут\n"; return str;}
		
		//Запись количества постов, пропускаемых в режиме #заткнись
		if (str_1 == "z" && flag_str_2) {PutInt ("silence_setup", digit_str_2); string str = "Количество пропускаемых постов в режиме #заткнись: " + to_string(digit_str_2) + "\n"; return str;}
		
		//Если первая подстрока число от 1 до 99
		//Вызов функции редактирования файлов FileWrite
		if (digit_str_1 > 0 && digit_str_1 < 100 && flag_str_1) {return FileWrite (digit_str_1, str_2,  str_3);}
		
		//Если не поступила ни одна из вышеперечисленных комад, то вывод файла ReadMe.txt
		if (!flag_str_1 && !flag_str_2 && !flag_str_3) return FileReadLinear ("ReadMe.txt");

return "Нет такой команды";
}



/* 
Функция подстановки ника пользователя для формирования ответа
 */
void SendingPost (string& str)
{
if (RandomNumber(1, 100) % 2 == 0)	arg_1 = str_first_name + ", " + str;		//Если случайное число четное, то подставляется ник
else								arg_1 = FirstCharUp (str);					//Если случайное число не четное, то первый символ в верхний регистр и без ника
Request(req_name, chat_id, arg_1, arg_2);										//Отправка ответа
}
