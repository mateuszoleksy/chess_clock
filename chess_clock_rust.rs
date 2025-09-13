#![no_std]
#![no_main]

use panic_halt as _; // panic handler
use rtic::app;

#[app(device = stm32f4xx_hal::stm32, peripherals = true)]
mod app {
    use super::*;
    use stm32f4xx_hal::prelude::*;

    // Shared resources między taskami
    #[shared]
    struct Shared {
        current_player: u8,
        time_left: [[u32; 2]; 2], // [player][minutes, milliseconds]
        pause: bool,
        set: bool,
        flashing: bool,
    }

    // Local resources dla tasków
    #[local]
    struct Local {
        sec: u32,
        blink: u32,
    }

    #[init]
    fn init(mut ctx: init::Context) -> (Shared, Local, init::Monotonics) {
        // inicjalizacja portów / GPIO
        let dp = ctx.device;
        // tu normalnie HAL GPIO init dla wyświetlacza i klawiatury

        let shared = Shared {
            current_player: 0,
            time_left: [[1, 0], [1, 0]],
            pause: false,
            set: false,
            flashing: false,
        };

        let local = Local {
            sec: 0,
            blink: 0,
        };

        // start timerów lub tasków (RTIC sam wywoła periodic taski)
        (shared, local, init::Monotonics())
    }

    // Task odliczający czas gracza, co 1 sekundę
    #[task(shared = [time_left, current_player, pause, set])]
    fn timer_task(ctx: timer_task::Context) {
        let shared = ctx.shared;

        shared.lock(|s| {
            if !s.pause && !s.set {
                if s.time_left[s.current_player as usize][1] >= 1000 {
                    s.time_left[s.current_player as usize][1] -= 1000;
                } else if s.time_left[s.current_player as usize][0] > 0 {
                    s.time_left[s.current_player as usize][0] -= 1;
                    s.time_left[s.current_player as usize][1] = 59000;
                }
            }
        });

        // RTIC: wywołanie ponownie za 1s (planowanie kolejnego tasku)
        timer_task::spawn_after(1_000_000u32.cycles()).ok();
    }

    // Task odświeżania wyświetlacza (co 20ms)
    #[task(shared = [time_left, current_player, flashing, set], local = [blink])]
    fn display_task(ctx: display_task::Context) {
        let blink = ctx.local.blink;

        ctx.shared.lock(|s| {
            if s.flashing {
                *blink += 1;
                if *blink >= 50 { *blink = 0; }
                // miganie wyświetlacza, np. DDRE = 0x00 / 0xFF analog
            }

            // Wyświetlanie czasu current_player
            let display_sec = s.time_left[s.current_player as usize][1] / 1000;
            let display_min = s.time_left[s.current_player as usize][0];

            // TODO: mapowanie digit -> segmenty i wyświetlanie
            // np. display_digit(display_sec % 10, ...);
        });

        display_task::spawn_after(20_000u32.cycles()).ok();
    }

    // Task odczytujący klawiaturę (co 50ms)
    #[task(shared = [current_player, pause, set, flashing, time_left])]
    fn keypad_task(ctx: keypad_task::Context) {
        ctx.shared.lock(|s| {
            // tu analog Twojego readKeypad() i logika przycisków
            // zmiana gracza, pauza, set, ustawianie czasu
        });

        keypad_task::spawn_after(50_000u32.cycles()).ok();
    }
}
