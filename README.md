# Firmware Contest — DS1307 I2C 題目 Template

## 專案結構

```
firmware-contest/
├── platformio.ini                  # 建置設定（STM32F103C8 + Arduino）
├── wokwi.toml                      # Wokwi CLI 設定
├── diagram.json                    # 情境 A：正常電路（DS1307 連接）
├── diagram-no-device.json          # 情境 B：無設備（I2C 浮空）
├── diagram-slow-device.json        # 情境 C：延遲回應（Custom Chip）
├── src/
│   └── main.cpp                    # Firmware 主程式（請在此實作）
├── chips/
│   ├── slow-ds1307.chip.json       # Custom Chip 描述
│   └── slow-ds1307.chip.c          # Custom Chip 實作（前 N 次 NACK）
└── .github/
    └── workflows/
        └── verify.yml              # CI：build + 三情境模擬驗證
```

---

## 題目說明

請實作 firmware，滿足以下三個情境的輸出規格。

### 情境 A：正常運作

DS1307 正常連接，I2C 讀取成功。

**預期輸出：**
```
BOOT OK
TIME 2001-02-03 04:05:06
```

### 情境 B：設備不存在

I2C bus 上無任何設備，所有讀取嘗試均收到 NACK。

**預期輸出：**
```
BOOT OK
RETRY 1/3 NACK
RETRY 2/3 NACK
RETRY 3/3 NACK
ERROR NACK
```

### 情境 C：設備延遲回應

設備前 2 次不回應（NACK），第 3 次才正常回應。

**預期輸出：**
```
BOOT OK
RETRY 1/3 NACK
RETRY 2/3 NACK
TIME 2001-02-03 04:05:06
```

---

## 輸出格式規格

| 輸出行 | 格式 | 說明 |
|--------|------|------|
| `BOOT OK` | 固定字串 | setup() 開始時輸出 |
| `RETRY n/N ERR` | `RETRY %d/%d %s` | 每次失敗後輸出，ERR 為錯誤代碼 |
| `TIME YYYY-MM-DD HH:MM:SS` | 固定格式 | 讀取成功時輸出 |
| `ERROR ERR` | `ERROR %s` | 全部 retry 失敗後輸出 |

錯誤代碼：
- `NACK`：address NACK（設備不存在或未就緒）
- `SHORT`：回傳 byte 數不足
- `ERR`：其他 I2C 錯誤

---

## 驗證方式

正式驗證以 GitHub Actions 為準。推送到 `main` 分支後，CI 會自動：

1. 使用 PlatformIO 編譯 firmware
2. 以三種 diagram 分別跑 Wokwi 模擬
3. 比對 serial output 是否符合格式規格
4. 上傳 serial log 作為 artifact

### 本機手動測試（Wokwi Web）

1. 前往 [wokwi.com](https://wokwi.com) 建立 STM32 Blue Pill 專案
2. 貼入對應情境的 `diagram.json`
3. 貼入 `src/main.cpp`
4. 點擊 **Play**，觀察右側 Serial Monitor

---

## 環境需求

- **本機**：不需安裝任何工具，可直接在 `github.dev` 編輯並推送
- **CI 環境**：GitHub Actions 自動安裝 PlatformIO 與 Wokwi CLI
- **Wokwi Token**：需在 repository Settings → Secrets 設定 `WOKWI_CLI_TOKEN`

### 設定 Wokwi CI Token

1. 登入 [wokwi.com](https://wokwi.com) → 右上角 → CI Tokens
2. 建立新 Token，複製字串
3. GitHub repository → Settings → Secrets and variables → Actions
4. 新增 secret：Name `WOKWI_CLI_TOKEN`，Value 貼上 token

---

## 硬體腳位

| 功能 | STM32 腳位 |
|------|-----------|
| I2C1 SDA | PB7 |
| I2C1 SCL | PB6 |
| USART1 TX | PA9 |
| USART1 RX | PA10 |

---

## 注意事項

- 正式成績以 GitHub 上指定 commit 的 CI 結果為準
- Wokwi web 手動模擬僅供開發測試用途，不計入正式成績
- 不建議修改 `diagram.json`、`chips/` 與 `.github/workflows/` 內容
- 允許修改：`src/main.cpp`、`include/`（可新增標頭檔）
