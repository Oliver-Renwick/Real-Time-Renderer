include "Dependencies.lua"

workspace "Real_Time_Renderer"
   configurations 
   { 
      "Debug", 
      "Release" 
   }

   platforms 
   { 
      "x64" 
   }     

project "APP"
   kind "WindowedApp"
   language "C++"
   cppdialect "C++20"

   targetdir "bin/%{cfg.buildcfg}"

   files 
   { 
       "APP/**.h",
       "APP/**.cpp" 
   }

   includedirs
   {
      "Vulkan_Renderer",
      "%{IncludeDir.VulkanSDK}",
      "%{IncludeDir.GLM}",
      "%{IncludeDir.KTX}",
      "%{IncludeDir.TINY_GLTF}",
      "%{IncludeDir.ImGUI}",
   } 

   

   links
   {
      "Vulkan_Renderer",
      "%{Library.Vulkan}",  
      "%{Library.ktx}"     
   }

   filter "system:windows"
      defines { 
         "VK_USE_PLATFORM_WIN32_KHR",
         "PROJECT_PATH=\"" .. os.getcwd() .. "\"" 
    } 

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

   filter "platforms:x64"  
      architecture "x86_64"



-- Static Library Stuff
project "Vulkan_Renderer"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"

   targetdir "bin/%{cfg.buildcfg}"

   files 
   { 
       "Vulkan_Renderer/**.h",
       "Vulkan_Renderer/**.cpp" 
   }

   includedirs
   {
      "Vulkan_Renderer",
      "%{IncludeDir.VulkanSDK}",
      "%{IncludeDir.GLM}",
      "%{IncludeDir.KTX}",
      "%{IncludeDir.TINY_GLTF}",
      "%{IncludeDir.ImGUI}",
   } 

 

   links
   {
       "%{Library.Vulkan}",  
       "%{Library.ktx}" 
   }

   filter "system:windows"
      defines { 
         "VK_USE_PLATFORM_WIN32_KHR",
         "PROJECT_PATH=\"" .. os.getcwd() .. "\"" 
    } 

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

   filter "platforms:x64"  
      architecture "x86_64"



