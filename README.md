Rozbuduj przedstawiony na wykładzie lub opisany w załączonych plikach (.cpp i .h w snake.zip) system okienkowy poprzez zaimplementowanie w nim planszy do gry „w węża” (Snake).

Minimalne wymagania co do rozbudowanej funkcjonalności:

    w momencie uruchomienia programu pojawiają się 3 przykładowe okna, w tym okno do gry (dynamiczne dodawanie/usuwanie okien jest dozwolone ale nie jest wymagane)
    rozmiary okien mogą być stałe (dynamiczna zmiana rozmiaru okien jest dozwolona ale nie jest wymagana)
    okno do gry obsługuje dowolne klawisze pozwalające na:
        wyświetlenie informacji pomocy
        wstrzymanie / wznowienie gry
        ponowne uruchomienie gry
        poruszanie wężem
        poruszanie oknem do gry
    opis znaczenia wybranych klawiszy pojawia się na początku gry w formie krótkiej instrukcji
    w przypadku, gdy gracz nie steruje wężem w trakcie gry, wąż samoczynnie porusza się w ostatnio wybranym kierunku
    wąż nie może w jednym ruchu wykonać zwrotu o 180 stopni
    przekroczenie przez węża jednej ze ścian gry przenosi go na przeciwległą ścianę
    na planszy umieszczany jest element symbolizujący pokarm dla węża - naprowadzenie węża na taki element powoduje wydłużenie się węża
    pozycja elementu symbolizującego pokarm dla węża jest wybierana pseudo-losowo
    wydłużanie się węża zwiększa poziom gry i szybkość samoczynnych ruchów węża 
    w czasie gry wyświetlana jest (na planszy lub nad planszą) informacja o aktualnym poziomie gry
    w momencie zakończenia gry wyświetlana jest (na planszy lub nad planszą) informacja o liczbie zdobytych punktów (osiągniętej długości węża)
