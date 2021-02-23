module.exports = [
    {
      "type": "heading",
      "defaultValue": "App Configuration"
    },
    {
      "type": "text",
      "defaultValue": "Here is some introductory text."
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "More Settings"
        },
        {
          "type": "toggle",
          "messageKey": "fahrenheit",
          "label": "Use Fahrenheit",
          "defaultValue": false
        }
      ]
    },
    {
      "type": "submit",
      "defaultValue": "Save Settings"
    }
  ];