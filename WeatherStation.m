s = serial('COM5','BAU',115200,'terminator','LF'); % Windows
%s = serial('/dev/cu.usbmodem141201','BAU',9600,'terminator','LF'); % MAC
fopen(s);
str = fscanf(s);
expression = '\d*.\d*,\d*.\d*,\d*.\d*,\d*.\d*';

while(isempty(regexp(str,expression,'once'))) % verifico che nel buffer ci siano i dati nel formato corretto
    pause(2); 
    str = fscanf(s);
end

arr = nan(300,4);
data = fscanf(s,'%f, %f, %f, %f').';
prev = data;
arr(1,:) = data;
i=2;

while 1 % ciclo do-while
    data = fscanf(s,'%f, %f, %f, %f').'; % acquisisco i valori in continuazione
    if(max(data ~= prev)==1) % se c'è almeno un valore diverso salvo la lettura
        arr(i,:) = data;
        prev = data;
        i=i+1;
    end
    if(i>300) % salvo solo 150 letture
        break;
    end
end

fclose(s);
data_media   = mean(arr); % array con la media dei valori 
data_mediana = median(arr); % array con la mediana dei valori
% decommentare se è la prima vota che si esegue il programma 
% crea la matrice dove vengono collezionati i dati
% weatherStationData_description = ["Time", "Temperatura [°C]", "Umidità [%]", "Pressione [Kpa]", "Elevazione [m]"];
% weatherStationData = [weatherStationData_description;string(datetime),data_media];

% commentare se è la prima volta che si esegue
% carica la matrice e aggiunge la misura
load('WeatherStationData.mat');
weatherStationData = [weatherStationData;string(datetime),data];

save("WeatherStationData.mat","weatherStationData");

% Calcolo media
media_temp = data_media(1); % prende la media solo della colonna 1 (temperatura)
media_umidita = data_media(2);
media_press = data_media(3);
media_elev = data_media(4);

%plot 4 grafici con le rispettive medie

% Traccia grafico temp
figure;
hold on;
plot(arr(:,1), 'b'); % traccia solo la colonna 1 (temperatura)
plot([1 300], [media_temp media_temp], 'r'); % media
hold off;
xlabel('Numero di misure');
ylabel('Temperatura [°C]');
legend('Valori di temperatura', 'Media');

% Traccia grafico umidità
figure;
hold on;
plot(arr(:,2), 'b'); % traccia solo la colonna 2 (umidità)
plot([1 300], [media_umidita media_umidita], 'r'); % media
hold off;
xlabel('Numero di misure');
ylabel('Umidità [%]');


% Traccia grafico pressione
figure;
hold on;
plot(arr(:,3), 'b'); % traccia solo la colonna 3 (pressione)
plot([1 300], [media_press media_press], 'r'); % media
hold off;
xlabel('Numero di misure');
ylabel('Pressione [Kpa]');
legend('Valori di pressione', 'Media');

% Traccia grafico elevazione
figure;
hold on;
plot(arr(:,4), 'b'); % traccia solo la colonna 4 (elevazione)
plot([1 300], [media_elev media_elev], 'r'); % media
hold off;
xlabel('Numero di misure');
ylabel('Elevazione [m]');
legend('Valori di elevazione', 'Media');


%%%%%%%%%% mediana temp %%%%%%%%%%%%%%


% Seleziono solo i valori di temperatura
temperatures = arr(1:end,1);

% Calcolo la mediana delle temperature
median_temperature = median(temperatures);
x = 1:length(temperatures);
figure;
% Plot dei valori di temperatura
plot(temperatures, 'b');
hold on;
% Plot della mediana
plot(x,median_temperature*ones(length(x),1), 'r', 'LineWidth', 2);

% Aggiungo una legenda
xlabel('Numero di misure');
ylabel('Temperatura [°C]');
legend('Temperatura', 'Mediana temperatura');

%%%%%%%% smothing w10 %%%%%%%%%%%%%

window_size = 10; % dimensione della finestra di filtraggio

filtered_data = zeros(1, length(temperatures)); % array per i dati filtrati

for i = 1:length(temperatures)
    start = max(1, i - (window_size - 1) / 2); % inizio della finestra di filtraggio
    stop = min(length(temperatures), i + (window_size - 1) / 2); % fine della finestra di filtraggio
    window = temperatures(start:stop); % finestra di filtraggio
    filtered_data(i) = median(window); % valore mediano della finestra
end

%%%%%%%%% plot smothing %%%%%%%%%%%%%

figure;
plot(temperatures, 'b'); % plotta i dati di temperatura originali
hold on;
plot(filtered_data, 'r'); % plotta i dati filtrati
legend('Temperatura originale', 'Temperatura filtrata w10');
xlabel('Numero di misure');
ylabel('Temperatura [°C]');


% Calcolo la mediana dei dati filtrati 
s_mediana = median(filtered_data); %mediana dei dati filtrati 

% plot dei dati filtrati con la mediana deei dati
figure;
plot(filtered_data, 'b');
hold on;
% Plot della mediana
plot(x, s_mediana*ones(length(x),1), 'r', 'LineWidth', 2);

% Aggiungo una legenda
xlabel('Numero di misure');
ylabel('Temperatura [°C]');
legend('Dati filtrati', 'Mediana');


weatherStationData(size(weatherStationData,1),2) = mean (filtered_data);

