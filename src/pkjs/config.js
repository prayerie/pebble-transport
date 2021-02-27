module.exports = [
    {
      "type": "heading",
      "defaultValue": "Watchface config"
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Colour scheme"
        },
        {
          "type": "radiogroup",
          "messageKey": "colourScheme",
          "label": "Colour scheme",
          "defaultValue": "red",
          "options": [
            { 
              "label": "White", 
              "value": "white"
            },
            { 
              "label": "Blue", 
              "value": "blue"
            },
            { 
              "label": "Green", 
              "value": "green"
            },
            { 
              "label": "Brown", 
              "value": "brown"
            },
            { 
              "label": "Black", 
              "value": "black"
            },
            { 
              "label": "Red", 
              "value": "red"
            },
            { 
              "label": "Yellow", 
              "value": "yellow"
            }
          ]
        }
      ]
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Settings"
        },
        {
          "type": "toggle",
          "messageKey": "fahrenheit",
          "label": "Use Fahrenheit",
          "defaultValue": false
        },
        {
          "type": "toggle",
          "messageKey": "fillCorners",
          "label": "Fill watchface corners",
          "defaultValue": false
        },
        {
          "type": "toggle",
          "messageKey": "twoLineWeather",
          "label": "Two-line weather (doesn't trunacte conditions)",
          "defaultValue": false
        }
      ]
    },
    {
      "type": "section",
      "items": [
        {
          "type": "input",
          "messageKey": "customKey",
          "defaultValue": "",
          "label": "Custom OpenWeatherMap API key",
          "attributes": {
            "placeholder": "Leave blank to use built-in key",
            "limit": 32,
          }
        },
        {
          "type": "input",
          "messageKey": "customLoc",
          "defaultValue": "",
          "label": "Override location",
          "attributes": {
            "placeholder": "Leave blank to use current GPS location",
            "limit": 32,
          }
        }
      ]
    },
    {
      "type": "submit",
      "defaultValue": "Save"
    }
  ];