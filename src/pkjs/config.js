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
        }
      ]
    },
    {
      "type": "submit",
      "defaultValue": "Save"
    }
  ];