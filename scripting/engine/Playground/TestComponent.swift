import Foundation

@component(.onStart, .onDestroy)
public class TestComponent: Component {
    var id: String = UUID().uuidString
    var health = 200

    public func onStart() {

    }

    public func onDestroy() {
        
    }
}
