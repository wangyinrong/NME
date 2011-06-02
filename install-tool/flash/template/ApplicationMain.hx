::foreach assets::
  ::if (type=="image")::
     class NME_::flatName:: extends flash.display.BitmapData { public function new() { super(10,10); } }
  ::else::
     class NME_::flatName:: extends ::flashClass:: { }
  ::end::
::end::




class ApplicationMain
{
   static var mPreloader:NMEPreloader;

   public static function main()
   {
      var call_real = true;
      ::if (PRELOADER_NAME!="")::
         var loaded:Int = flash.Lib.current.loaderInfo.bytesLoaded;
         var total:Int = flash.Lib.current.loaderInfo.bytesTotal;
         if (loaded<total)
         {
            call_real = false;
            mPreloader = new ::PRELOADER_NAME::();
            flash.Lib.current.addChild(mPreloader);
            mPreloader.onInit();
            mPreloader.onUpdate(loaded,total);
            flash.Lib.current.addEventListener(flash.events.Event.ENTER_FRAME, onEnter);
         }
      ::end::

      if (call_real)
         ::APP_MAIN::.main();
   }

   static function onEnter(_)
   {
      var loaded:Int = flash.Lib.current.loaderInfo.bytesLoaded;
      var total:Int = flash.Lib.current.loaderInfo.bytesTotal;
      mPreloader.onUpdate(loaded,total);
      if (loaded>=total)
      {
         mPreloader.onLoaded();
         flash.Lib.current.removeEventListener(flash.events.Event.ENTER_FRAME, onEnter);
         flash.Lib.current.removeChild(mPreloader);
         mPreloader = null;

         ::APP_MAIN::.main();
      }
   }

   public static function getAsset(inName:String) : Dynamic
   {
      ::foreach assets::
      if (inName=="::id::")
         ::if (flashClass=="flash.utils.ByteArray")::
      {
         var obj = new NME_::flatName::();
         return haxe.io.Bytes.ofData( obj );
      }
         ::else::
         return new NME_::flatName::();
         ::end::
      ::end::

      return null;
   }
}