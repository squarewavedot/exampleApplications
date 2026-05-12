============================
Dynamic Parameter System
============================

The matrixapplication framework (version 0.4.0+) introduces a powerful Dynamic Parameter System designed to allow real-time configuration of running matrix applications.

This system bridges the gap between the C++ backend and the new **CubeWebapp** frontend, allowing applications to expose adjustable parameters (like speed, color, spawn rate, etc.) automatically to the web interface.

Core Concepts
=============

The parameter system consists of several levels:

1. **AnimationParams**: A C++ class that holds the schema and current values for an application's parameters.
2. **MatrixApplication Integration**: The base class for all applications seamlessly handles incoming network requests for parameter updates and queries.
3. **Protobuf Messages**: Data definition language used for transmitting schema and values over WebSockets.
4. **CubeWebapp UI**: The web frontend that dynamically generates UI controls based on the application's configuration.

How to Use
==========

To expose parameters in your custom application, follow these steps:

1. Register Parameters in the Constructor
---------------------------------------

Within your application's constructor (derived from ``CubeApplication`` or ``MatrixApplication``), register the parameters you wish to expose. They must be registered before the application's main loop begins.

.. code-block:: cpp

    MyApplication::MyApplication() : CubeApplication("MyApplication") {
        // Register an integer parameter
        params.registerIntParam("Speed", "Animation Speed", 1, 10, 5);
        
        // Register a float parameter
        params.registerFloatParam("Gravity", "Gravity Multiplier", 0.1f, 2.0f, 1.0f);
        
        // Register a boolean parameter
        params.registerBoolParam("EnableSparkles", "Enable Sparkles", true);
        
        // Register an enum (dropdown) parameter
        params.registerEnumParam("ColorMode", "Color Mode", {"Rainbow", "Solid", "Pulse"}, "Rainbow");
    }

2. Read Parameters in Your Loop
-------------------------------

During your application's ``tick()`` or main rendering loop, retrieve the current values. Accesses are thread-safe and will immediately reflect any changes made by the user in the CubeWebapp UI.

.. code-block:: cpp

    void MyApplication::tick() {
        int speed = params.getIntParam("Speed");
        float gravity = params.getFloatParam("Gravity");
        bool sparkles = params.getBoolParam("EnableSparkles");
        std::string mode = params.getStringParam("ColorMode");

        // Use parameters to drive animation
        // ...
    }

Supported Parameter Types
=========================

* **Integer**: ``registerIntParam(id, name, min, max, default)`` - Rendered as a slider in the UI.
* **Float**: ``registerFloatParam(id, name, min, max, default)`` - Rendered as a fine-grained slider.
* **Boolean**: ``registerBoolParam(id, name, default)`` - Rendered as a checkbox.
* **String/Enum**: ``registerEnumParam(id, name, options_vector, default)`` - Rendered as a dropdown selector.

Presets
=======

The CubeWebapp frontend includes a robust preset management system. Users can save combinations of parameter configurations as JSON files across different applications. When a preset is loaded, the frontend transmits the saved values to the active C++ application automatically, instantly updating its state without requiring a restart.

Network Communication
=====================

Parameter synchronization happens transparently via the ``matrixserver`` network protocol. When the ``CubeWebapp`` requests the active schema, the server queries the running application via an ``appParamValues`` message. Parameter changes from the web interface are dispatched as ``setAppParam`` commands, which safely update the ``AnimationParams`` object in the C++ backend.
