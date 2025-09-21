package main

import (
	"fmt"
	"log"
	"os"
	"strconv"
	"time"

	tgbotapi "github.com/go-telegram-bot-api/telegram-bot-api/v5"
)

var bot *tgbotapi.BotAPI

type UserState struct {
	Step string
	City string
	Date string
	Data WeatherData
}

type WeatherData struct {
	City        string
	Date        string
	Temperature float64
	Sun         float64
	UV          float64
	Humidity    float64
	Wind        float64
}

var userStates = make(map[int64]*UserState)

var userData = make(map[int64][]WeatherData)

func main() {
	token := os.Getenv("WEATHER_TOKEN")
	if token == "" {
		log.Fatal("TELEGRAM_BOT_TOKEN is required")
	}

	var err error
	bot, err = tgbotapi.NewBotAPI(token)
	if err != nil {
		log.Fatal(err)
	}

	log.Printf("Authorized on %s", bot.Self.UserName)

	u := tgbotapi.NewUpdate(0)
	u.Timeout = 30
	updates := bot.GetUpdatesChan(u)

	for update := range updates {
		if update.Message != nil {
			handleMessage(update.Message)
		} else if update.CallbackQuery != nil {
			handleCallback(update.CallbackQuery)
		}
	}
}

func sendMessage(chatID int64, text string) {
	msg := tgbotapi.NewMessage(chatID, text)
	msg.ParseMode = "Markdown"
	bot.Send(msg)
}

func sendMessageWithMenu(chatID int64, text string) {
	kb := tgbotapi.NewInlineKeyboardMarkup(
		tgbotapi.NewInlineKeyboardRow(
			tgbotapi.NewInlineKeyboardButtonData("⬅️ В главное меню", "main_menu"),
		),
	)
	msg := tgbotapi.NewMessage(chatID, text)
	msg.ParseMode = "Markdown"
	msg.ReplyMarkup = kb
	bot.Send(msg)
}

func sendStartMenu(chatID int64) {
	text := "*🌤 Добро пожаловать в Анализатор Погодных Данных!*\n\nВыберите действие:"
	kb := tgbotapi.NewInlineKeyboardMarkup(
		tgbotapi.NewInlineKeyboardRow(
			tgbotapi.NewInlineKeyboardButtonData("📥 Ввести данные по городу", "choose_city"),
			tgbotapi.NewInlineKeyboardButtonData("📊 Просмотреть статистику", "view_stats"),
		),
		tgbotapi.NewInlineKeyboardRow(
			tgbotapi.NewInlineKeyboardButtonData("ℹ О программе", "about"),
		),
	)
	msg := tgbotapi.NewMessage(chatID, text)
	msg.ParseMode = "Markdown"
	msg.ReplyMarkup = kb
	bot.Send(msg)
}

func sendCityMenu(chatID int64) {
	cities := []struct{ Name, Code string }{
		{"Минск", "Minsk"}, {"Гомель", "Gomel"}, {"Могилёв", "Mogilev"},
		{"Витебск", "Vitebsk"}, {"Гродно", "Grodno"}, {"Брест", "Brest"},
		{"Барановичи", "Baranovichi"}, {"Борисов", "Borisov"}, {"Пинск", "Pinsk"},
		{"Орша", "Orsha"}, {"Мозырь", "Mozyr"}, {"Солигорск", "Soligorsk"},
		{"Новополоцк", "Novopolotsk"}, {"Лида", "Lida"}, {"Молодечно", "Molodechno"},
		{"Полоцк", "Polotsk"}, {"Жлобин", "Zhlobin"}, {"Светлогорск", "Svetlogorsk"},
		{"Речица", "Rechitsa"}, {"Жодино", "Zhodino"},
	}

	var rows [][]tgbotapi.InlineKeyboardButton
	for _, city := range cities {
		rows = append(rows, tgbotapi.NewInlineKeyboardRow(
			tgbotapi.NewInlineKeyboardButtonData(city.Name, "city_"+city.Code),
		))
	}
	rows = append(rows, tgbotapi.NewInlineKeyboardRow(
		tgbotapi.NewInlineKeyboardButtonData("⬅️ Назад", "main_menu"),
	))

	kb := tgbotapi.NewInlineKeyboardMarkup(rows...)
	msg := tgbotapi.NewMessage(chatID, "Выберите город:")
	msg.ReplyMarkup = kb
	bot.Send(msg)
}

func sendDateMenu(chatID int64, city string) {
	dates := []string{
		time.Now().Format("2006-01-02"),
		time.Now().AddDate(0, 0, 1).Format("2006-01-02"),
		time.Now().AddDate(0, 0, 2).Format("2006-01-02"),
	}

	var rows [][]tgbotapi.InlineKeyboardButton
	for _, d := range dates {
		rows = append(rows, tgbotapi.NewInlineKeyboardRow(
			tgbotapi.NewInlineKeyboardButtonData(d, "date_"+city+"_"+d),
		))
	}
	rows = append(rows, tgbotapi.NewInlineKeyboardRow(
		tgbotapi.NewInlineKeyboardButtonData("⬅️ Назад", "choose_city"),
	))

	kb := tgbotapi.NewInlineKeyboardMarkup(rows...)
	msg := tgbotapi.NewMessage(chatID, "Выберите дату для города "+city+":")
	msg.ReplyMarkup = kb
	bot.Send(msg)
}

func handleWeatherInput(msg *tgbotapi.Message, state *UserState) {
	text := msg.Text
	switch state.Step {
	case "input_temperature":
		val, err := strconv.ParseFloat(text, 64)
		if err != nil {
			sendMessage(msg.Chat.ID, "Введите число корректно.")
			return
		}
		state.Data.Temperature = val
		state.Step = "input_sun"
		sendMessage(msg.Chat.ID, "Введите солнечную радиацию (Вт/м²):")
	case "input_sun":
		val, err := strconv.ParseFloat(text, 64)
		if err != nil {
			sendMessage(msg.Chat.ID, "Введите число корректно.")
			return
		}
		state.Data.Sun = val
		state.Step = "input_uv"
		sendMessage(msg.Chat.ID, "Введите UV индекс:")
	case "input_uv":
		val, err := strconv.ParseFloat(text, 64)
		if err != nil {
			sendMessage(msg.Chat.ID, "Введите число корректно.")
			return
		}
		state.Data.UV = val
		state.Step = "input_humidity"
		sendMessage(msg.Chat.ID, "Введите влажность (%):")
	case "input_humidity":
		val, err := strconv.ParseFloat(text, 64)
		if err != nil {
			sendMessage(msg.Chat.ID, "Введите число корректно.")
			return
		}
		state.Data.Humidity = val
		state.Step = "input_wind"
		sendMessage(msg.Chat.ID, "Введите скорость ветра (м/с):")
	case "input_wind":
		val, err := strconv.ParseFloat(text, 64)
		if err != nil {
			sendMessage(msg.Chat.ID, "Введите число корректно.")
			return
		}
		state.Data.Wind = val
		state.Step = ""
		state.Data.City = state.City
		state.Data.Date = state.Date

		// Сохраняем данные
		userData[msg.Chat.ID] = append(userData[msg.Chat.ID], state.Data)

		sendMessage(msg.Chat.ID, "✅ Данные сохранены!")
		sendStartMenu(msg.Chat.ID)
		delete(userStates, msg.Chat.ID)
	}
}

func handleMessage(msg *tgbotapi.Message) {
	state, exists := userStates[msg.Chat.ID]
	if exists && state.Step != "" {
		handleWeatherInput(msg, state)
		return
	}

	if msg.Text == "/start" {
		sendStartMenu(msg.Chat.ID)
	} else {
		sendStartMenu(msg.Chat.ID)
	}
}

func handleCallback(cb *tgbotapi.CallbackQuery) {
	bot.Request(tgbotapi.NewCallback(cb.ID, ""))
	chatID := cb.Message.Chat.ID
	data := cb.Data

	switch {
	case data == "main_menu":
		sendStartMenu(chatID)
	case data == "choose_city":
		sendCityMenu(chatID)
	case data[:5] == "city_":
		city := data[5:]
		userStates[chatID] = &UserState{Step: "", City: city}
		sendDateMenu(chatID, city)
	case data[:5] == "date_":
		info := data[5:]
		var city, date string
		fmt.Sscanf(info, "%[^_]_%s", &city, &date)
		state := &UserState{
			Step: "input_temperature",
			City: city,
			Date: date,
		}
		userStates[chatID] = state
		sendMessage(chatID, "Введите температуру (°C):")
	case data == "view_stats":
		showStatistics(chatID)
	case data == "about":
		sendMessageWithMenu(chatID, "*ℹ О программе*\n\nАнализатор Погодных Данных позволяет отслеживать и анализировать показатели погоды.")
	}
}

func showStatistics(chatID int64) {
	data, ok := userData[chatID]
	if !ok || len(data) == 0 {
		sendMessage(chatID, "Нет введённых данных.")
		return
	}

	result := "📊 Введённые данные:\n"
	for _, d := range data {
		result += fmt.Sprintf(
			"• %s %s: 🌡 %.1f°C, ☀️ %.1f Вт/м², UV %.1f, 💧 %.1f%%, 🌬 %.1f м/с\n",
			d.City, d.Date, d.Temperature, d.Sun, d.UV, d.Humidity, d.Wind,
		)
	}

	sendMessage(chatID, result)
}
