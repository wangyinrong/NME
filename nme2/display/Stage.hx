package nme2.display;

import nme2.events.MouseEvent;
import nme2.geom.Point;

class Stage extends nme2.display.DisplayObjectContainer
{
   public var onKey: Int -> Bool -> Int -> Int ->Void; 
   public var onMouseButton: Int -> Int -> Int -> Bool -> Int ->Void; 
   public var onResize: Int -> Int ->Void; 
   public var onRender: Void ->Void; 
   public var onQuit: Void ->Void; 


   public function new(inHandle:Dynamic)
   {
      super(inHandle);
      nme_set_stage_handler(nmeHandle,nmeProcessStageEvent);
   }

   function nmeOnMouseMove(inEvent:Dynamic)
   {
      var obj:DisplayObject = nmeFindByID(inEvent.id);
      var local = obj.globalToLocal( new Point(inEvent.x, inEvent.y) );
      var event = MouseEvent.nmeCreate(MouseEvent.MOUSE_MOVE,inEvent,local);
   }
   

   function nmeProcessStageEvent(inEvent:Dynamic) : Dynamic
   {
      //trace(inEvent);
      // TODO: timer event?
      nme2.Manager.pollTimers();
      switch(Std.int(Reflect.field( inEvent, "type" ) ) )
      {
         case 1: // KEY
            if (onKey!=null)
               untyped onKey(inEvent.code, inEvent.down, inEvent.char, inEvent.flags );

         case 2: // MOUSE_MOVE
            nmeOnMouseMove(inEvent);

         case 3: // MOUSE_BUTTON
            if (onMouseButton!=null)
               onMouseButton(inEvent.button, inEvent.x, inEvent.y, inEvent.down, inEvent.flags);
         case 4: // RESIZE
            if (onResize!=null)
               untyped onResize(inEvent.x, inEvent.y);
            nme_render_stage(nmeHandle);

         case 5: // RENDER
            if (onRender!=null)
               untyped onRender();
            nme_render_stage(nmeHandle);

         case 6: // QUIT
            if (onQuit!=null)
               untyped onQuit();

         // TODO: user, sys_wm, sound_finished
      }

      return null;
   }

   static var nme_set_stage_handler = nme2.Loader.load("nme_set_stage_handler",2);
   static var nme_render_stage = nme2.Loader.load("nme_render_stage",1);
}
