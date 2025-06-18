import requests
import tkinter as tk
import time
import serial
s=serial.Serial("COM5",115200)
def get_weather(canvas):
    api='your api key'
    base_url=''#base url for making api requests
    city=textfield.get()
    url=f"{base_url}/weather?q={city}&appid={api}&units=metric" # construct the api  request url
    response=requests.get(url) # send the get request to api
    data=response.json() # parse the response to json format
    #print(data)
    weather_types=['thunderstorm with light rain','thunderstorm with rain','thunderstorm with heavy rain','light thunderstorm','thunderstorm','heavy thunderstorm','ragged thunderstorm','	thunderstorm with light drizzle',
                   'thunderstorm with drizzle','thunderstorm with heavy drizzle','light intensity drizzle','drizzle',
                   'heavy intensity drizzle','light intensity drizzle rain','drizzle rain','heavy intensity drizzle rain',
                   'shower rain and drizzle','heavy shower rain and drizzle','shower drizzle','light rain',
                   'moderate rain','heavy intensity rain','very heavy rain','extreme rain','freezing rain',
                   'light intensity shower rain','shower rain','heavy intensity shower rain','ragged shower rain',
                   'heavy snow','shower sleet','light rain and snow','rain and snow','light shower snow','shower snow',
                   'heavy shower snow','broken clouds','overcast clouds']
    if data['cod']=='404':
        print(f'invalid city {city} , please check your city name')
    else:
        curr_weather=data['weather'][0]['description']
        if curr_weather in weather_types:
            print("close dome")
            s.write(b'a')
            time.sleep(1000)
        else:
            print("open dome")
            s.write(b'b')
            
        
        humidity=data['main']['humidity']
        city_temp=((data['main']['temp']))
        min_temp=((data['main']['temp_min']))
        max_temp=((data['main']['temp_max']))
        pressure=data['main']['pressure']
        final_info=curr_weather+"\n"+str(city_temp)+"°C"
        final_data="\n"+"maximum temperature : "+str(max_temp)+"°C"+"\n"+"minimumm temperature : "+str(min_temp)+"°C"+"\n"+"humidity: "+str(humidity)+"\n"+"pressure: "+str(pressure)
        label1.config(text=final_info)
        label2.config(text=final_data)
canvas=tk.Tk()
canvas.geometry("600x500")
canvas.title("weather App")
f=("poppins",15,"bold")
t=("poppins",35,"bold")
textfield=tk.Entry(canvas,font=t)
textfield.pack(pady=20)
textfield.focus()
textfield.bind('<Return>',get_weather)
label1=tk.Label(canvas,font=t)
label1.pack()
label2=tk.Label(canvas,font=f)
label2.pack()
canvas.mainloop()
s.close()
